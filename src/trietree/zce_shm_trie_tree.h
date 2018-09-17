/*!
* @copyright  2004-2018  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_trie_tree.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2018年9月9日
* @brief      一个字典树(OR前缀树)的实现。字典树在在HW的计费的时候实现过。当时主要为了做计费。
*             时隔10年后，在互动影视有发现了一次运用场景。而且我按目标来看。可能也需要一个长度
*             限制。才好用。
*             
* @details    
*             
*             
*
* @note       
*       
*   《橄榄树》 齐豫
*   不要问我从哪里来
*   我的故乡在远方
*   为什么流浪
*   流浪远方 流浪
*   为了天空飞翔的小鸟
*   为了山间轻流的小溪
*   为了宽阔的草原
*   流浪远方 流浪
*   还有还有
*   为了梦中的橄榄树 橄榄树
*   不要问我从哪里来
*   我的故乡在远方
*   为什么流浪
*   为什么流浪远方
*   为了我梦中的橄榄树
*   不要问我从哪里来
*   我的故乡在远方
*   为什么流浪
*   流浪远方 流浪 [4]
*
*/

#ifndef ZCE_LIB_SHARE_MEM_TRIE_TREE_H_
#define ZCE_LIB_SHARE_MEM_TRIE_TREE_H_



namespace ZCE_LIB
{

//============================================================================================

/*!
@brief      TRIE TREE数据区的头部，用于存放此次，对象数量，等数据
            使用的NODE数量，FREE的NODE数量等
*/
struct _shm_trie_tree_head
{
    //我不能对模版搞个友元，算了，开始开放出来把
public:

    ///内存区的长度
    size_t         size_of_mmap_ = 0;
    ///NODE结点个数
    size_t         num_of_node_ = 0;

    ///FREE的NODE个数
    size_t         size_free_node_ = 0;
    ///USE的NODE个数
    size_t         size_use_node_ = 0;

    //空闲的NODE列表的头指针
    int32_t        freenode_list_next_ = -1;
};

//============================================================================================

///TRIE TREE的树形NODE的索引
template <class _letter_type> class _shm_trie_tree_node
{
public:

    ///
    _letter_type  letter_;
    ///兄节点
    int32_t    idx_brother_ = (-1);
    ///子节点
    int32_t    idx_children_ = (-1);
    ///
    int32_t    idx_father_ = (-1);
};


//============================================================================================
/*!
@brief      字典树(OR前缀树)，可以在共享内存（普通内存也行），一块内存中，使用，
            也可以多进程共享，（当然同步层面的事情，你自己考虑）

            额外需要的地址空间大小说明，每个node额外需要2个shm_index_t大小的空间，

@tparam     _meta_type 元类型，每个NODE包括的数据
*/
template <class _letter_type,
    class _equal_letter = std::equal_to<_letter_type> > class smem_trie_tree
{
protected:

    //index区要增加两个数据,一个表示root,
    static const int32_t ADDED_NUM_OF_NODE = 1;

    //index 1为 ROOT
    static const int32_t ROOT_NODE_INDEX = 0;

    //空序号指针标示,32位为0xFFFFFFFF,
    static const int32_t  _INVALID_NODE_INDEX = -1;

    typedef _shm_trie_tree_node<_letter_type> shm_trie_tree_node;
protected:

    //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
    //内存基础地址
    char                        *smem_base_;

    //LIST的头部区指针
    _shm_trie_tree_head         *trietree_head_;

    //数据区起始指针,
    shm_trie_tree_node          *node_base_;

    //根节点指针,第1个索引位表示
    shm_trie_tree_node          *root_node_;

protected:

    ///如果在共享内存使用,没有new,所以统一用initialize 初始化
    ///这个函数,不给你用,就是不给你用
    smem_trie_tree<_letter_type, _equal_letter>(int32_t num_node, void *pmmap, bool if_restore):
        smem_base_(pmmap),
        trietree_head_(NULL),
        node_base_(NULL),
        root_node_(NULL)
    {
    }

    smem_trie_tree<_letter_type, _equal_letter>():
        smem_base_(NULL),
        trietree_head_(NULL),
        node_base_(NULL),
        root_node_(NULL)
    {
    }

    ~smem_trie_tree<_letter_type, _equal_letter>()
    {
    }

    //只定义,不实现,
    const smem_trie_tree<_letter_type, _equal_letter> & operator=(
        const smem_trie_tree<_letter_type, _equal_letter> &others) = delete;

protected:

    //分配一个NODE,将其从FREELIST中取出
    int32_t create_node(const _letter_type &val)
    {
        //如果没有空间可以分配
        if (trietree_head_->size_free_node_ == 0)
        {
            return _INVALID_NODE_INDEX;
        }

        //从链上取1个下来
        int32_t free_idx = trietree_head_->freenode_list_next_;

        //从FREE链表上摘下来
        trietree_head_->freenode_list_next_ = (root_node_ + free_idx)->idx_brother_;
        //清理干净
        (node_base_ + free_idx)->idx_brother_ = _INVALID_NODE_INDEX;
        (node_base_ + free_idx)->idx_children_ = _INVALID_NODE_INDEX;
        (node_base_ + free_idx)->idx_father_ = _INVALID_NODE_INDEX;

        //用placement new生产对象,这个要不要更加严谨一点。
        new ( &((node_base_ + free_idx)->letter_)) _letter_type(val) ;

        trietree_head_->size_use_node_  ++;
        trietree_head_->size_free_node_ --;

        return free_idx;
    }
    //释放一个NODE,将其归还给FREELIST
    void destroy_node(int32_t pos)
    {
        size_t free_next = trietree_head_->freenode_list_next_;

        (root_node_ + pos)->idx_brother_ = free_next;
        (node_base_ + pos)->idx_children_ = _INVALID_NODE_INDEX;
        (node_base_ + pos)->idx_father_ = _INVALID_NODE_INDEX;
        trietree_head_->freenode_list_next_ = pos;

        //调用显式的析构函数
        (node_base_ + pos)->~_letter_type();

        trietree_head_->size_use_node_  --;
        trietree_head_->size_free_node_ ++;

    }

public:

    //内存区的构成为 头部区,NODE区，返回所需要的长度,
    static std::size_t alloc_size(const int32_t num_node)
    {
        //NODE的数量为 N+1，1为root
        return  sizeof(_shm_trie_tree_head)  + 
            sizeof(_shm_trie_tree_node<_letter_type> ) * (num_node + ADDED_NUM_OF_NODE) ;
    }

    smem_trie_tree<_letter_type, _equal_letter>* getinstance()
    {
        return this;
    }

    //初始化
    static smem_trie_tree<_letter_type, _equal_letter>* initialize(const int32_t num_node, char *pmmap, bool if_restore = false)
    {
        assert(pmmap!=NULL && num_node >0 );
        _shm_trie_tree_head *treehead = reinterpret_cast<_shm_trie_tree_head *>(pmmap);

        //如果是恢复,数据都在内存中,
        if (if_restore == true)
        {
            //检查一下恢复的内存是否正确,
            if (alloc_size(num_node) != treehead->size_of_mmap_ ||
                num_node != treehead->num_of_node_ )
            {
                return NULL;
            }
        }

        //初始化尺寸
        treehead->size_of_mmap_ = alloc_size(num_node);
        treehead->num_of_node_ = num_node;

        smem_trie_tree<_letter_type, _equal_letter>* instance = new smem_trie_tree<_letter_type>();

        //所有的指针都是更加基地址计算得到的,用于方便计算,每次初始化会重新计算
        instance->smem_base_ = pmmap;
        instance->trietree_head_ = treehead;
        instance->node_base_ = reinterpret_cast<shm_trie_tree_node *>(pmmap + sizeof(_shm_trie_tree_head));
        instance->root_node_ = instance->node_base_;
        //
        if (if_restore == false)
        {
            //清理初始化所有的内存,所有的节点为FREE
            instance->clear();
        }

        assert(treehead->size_use_node_ + treehead->size_free_node_ == treehead->num_of_node_);

        //打完收工
        return instance;
    }

    //清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理2个关键Node,以及相关长度,开始所有的数据是free.
        trietree_head_->size_free_node_ = trietree_head_->num_of_node_;
        trietree_head_->size_use_node_ = 0;

        root_node_->idx_brother_ = _INVALID_NODE_INDEX;
        root_node_->idx_children_ = _INVALID_NODE_INDEX;
        root_node_->idx_father_ = _INVALID_NODE_INDEX;

        //第一个是root，第二个就是FREE的
        trietree_head_->freenode_list_next_ = ADDED_NUM_OF_NODE;
        //初始化free数据区，用idx_brother_串起来
        for (int32_t i = 1; i < trietree_head_->num_of_node_ ; ++i )
        {
            node_base_[i].idx_brother_ = i+1;
            node_base_[i].idx_children_ = _INVALID_NODE_INDEX;
            node_base_[i].idx_father_ = _INVALID_NODE_INDEX;
        }
        node_base_[trietree_head_->num_of_node_].idx_brother_ = _INVALID_NODE_INDEX;
        node_base_[trietree_head_->num_of_node_].idx_children_ = _INVALID_NODE_INDEX;
        node_base_[trietree_head_->num_of_node_].idx_father_ = _INVALID_NODE_INDEX;
    }

    //
    bool empty()
    {
        if (trietree_head_->size_free_node_ == trietree_head_->num_of_node_)
        {
            return true;
        }

        return false;
    }
    //在插入数据前调用,这个函数检查
    bool full()
    {
        if (trietree_head_->size_free_node_ == 0 )
        {
            return true;
        }
        return false;
    };

    //求交集
    bool has_word(const _letter_type *ward, size_t word_len, size_t *matching_len)
    {

        int32_t find_note = root_node_->idx_children_;

        _equal_letter   equal_meta;
        size_t n = 0;
        while (_INVALID_NODE_INDEX != find_note && n < word_len)
        {
            //找到一个匹配的，寻找下一个
            bool if_find = false;
            while (_INVALID_NODE_INDEX != find_note  &&
                   false == (if_find = equal_meta(*(ward + n), node_base_[find_note].letter_)))
            {
                find_note = node_base_[find_note].idx_brother_;
            }
            if (if_find)
            {
                ++n;
                find_note = node_base_[find_note].idx_children_;
            }
        }
        //返回匹配长度
        *matching_len = n;
        //完全匹配
        if (n == word_len)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    //
    bool insert_word(const _letter_type *ward, size_t word_len )
    {
        int32_t pre_father = ROOT_NODE_INDEX;
        int32_t pre_brother = _INVALID_NODE_INDEX;
        int32_t find_note = root_node_->idx_children_;

        _equal_letter   equal_meta;
        size_t n = 0;
        while ( n < word_len)
        {
            if (_INVALID_NODE_INDEX == find_note)
            {
                int32_t new_node = create_node(*(ward+n));
                //没空间了。
                if (new_node == _INVALID_NODE_INDEX)
                {
                    return false;
                }
                //插入
                //根据上一个查询的内容决定如果挂接
                if (_INVALID_NODE_INDEX != pre_brother)
                {
                    node_base_[pre_brother].idx_brother_ = new_node;
                }
                else
                {
                    node_base_[pre_father].idx_children_ = new_node;
                }
                node_base_[pre_father].idx_father_ = pre_father;
                ++n;
                pre_father = new_node;
                find_note = node_base_[new_node].idx_children_;
                pre_brother = _INVALID_NODE_INDEX;
                //find_note 应该就是_INVALID_NODE_INDEX
                assert(find_note == _INVALID_NODE_INDEX);
            }
            else
            {
                //找到一个匹配的，寻找下一个
                bool if_find = false;
                //先把兄弟都找完，看是否有匹配的
                while (_INVALID_NODE_INDEX != find_note &&
                    false == (if_find = equal_meta(*(ward + n), node_base_[find_note].letter_)))
                {
                    pre_brother = find_note;
                    find_note = node_base_[find_note].idx_brother_;
                }
                if (if_find)
                {
                    ++n;
                    pre_father = find_note;
                    pre_brother = _INVALID_NODE_INDEX;
                    find_note = node_base_[find_note].idx_children_;
                }
            }
        }
        return true;
    }

    //发现里面是否有一个单词
    bool has_letter(const _letter_type &letter, size_t *letter_num)
    {
        size_t find_num = 0;
        int32_t find_note = root_node_->idx_children_;

        std::vector<int32_t> non_recursive_stack;
        non_recursive_stack.reserve(1024);
        non_recursive_stack.push_back(find_note);

        _equal_letter   equal_letter;
        while (non_recursive_stack.size())
        {
            find_note = non_recursive_stack.back();
            non_recursive_stack.pop_back();

            //找到一个匹配的，寻找下一个
            while (_INVALID_NODE_INDEX != find_note )
            {
                if (equal_letter(letter, node_base_[find_note].letter_))
                {
                    ++find_num;
                }
                if (_INVALID_NODE_INDEX != node_base_[find_note].idx_children_)
                {
                    non_recursive_stack.push_back(node_base_[find_note].idx_children_);
                }
                find_note = node_base_[find_note].idx_brother_;
            }
        }

        *letter_num = find_num;
        if (find_num > 0)
        {
            return true;
        }
        return false;
    }

    //返回链表中已经有的元素个数
    size_t size()
    {
        return trietree_head_->size_use_node_;
    }
    //返回链表池子的容量
    size_t capacity()
    {
        return trietree_head_->num_of_node_;
    }
    //
    size_t free()
    {
        return trietree_head_->size_free_node_;
    }

    //递归把所有的数据都dump出来看看。避免问题
    void dump_tree()
    {
        dump_node(ROOT_NODE_INDEX);
    }

    void dump_node(int32_t index)
    {
        std::cout << index << ":[" << node_base_[index].idx_brother_ << "," 
            << node_base_[index].idx_children_ << ","
            << node_base_[index].idx_father_ << ",";
        //正式环境注释掉这个，
        std::cout << node_base_[index].letter_ ;
        std::cout << "]" << std::endl;

        int32_t brother_note = node_base_[index].idx_brother_;
        int32_t children_node = node_base_[index].idx_children_;
        if (_INVALID_NODE_INDEX != children_node)
        {
            dump_node(children_node);
        }

        if (_INVALID_NODE_INDEX != brother_note)
        {
            dump_node(brother_note);
        }
    }

};

};

#endif //ZCE_LIB_SHARE_MEM_TRIE_TREE_H_

