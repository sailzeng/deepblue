/*!
* @copyright  2004-2018  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_trie_tree.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2018��9��9��
* @brief      һ���ֵ���(ORǰ׺��)��ʵ�֡��ֵ�������HW�ļƷѵ�ʱ��ʵ�ֹ�����ʱ��ҪΪ�����Ʒѡ�
*             ʱ��10����ڻ���Ӱ���з�����һ�����ó����������Ұ�Ŀ������������Ҳ��Ҫһ������
*             ���ơ��ź��á�
*             
* @details    
*             
*             
*
* @note       
*       
*   ��������� ��ԥ
*   ��Ҫ���Ҵ�������
*   �ҵĹ�����Զ��
*   Ϊʲô����
*   ����Զ�� ����
*   Ϊ����շ����С��
*   Ϊ��ɽ��������СϪ
*   Ϊ�˿����Ĳ�ԭ
*   ����Զ�� ����
*   ���л���
*   Ϊ�����е������ �����
*   ��Ҫ���Ҵ�������
*   �ҵĹ�����Զ��
*   Ϊʲô����
*   Ϊʲô����Զ��
*   Ϊ�������е������
*   ��Ҫ���Ҵ�������
*   �ҵĹ�����Զ��
*   Ϊʲô����
*   ����Զ�� ���� [4]
*
*/

#ifndef ZCE_LIB_SHARE_MEM_TRIE_TREE_H_
#define ZCE_LIB_SHARE_MEM_TRIE_TREE_H_



namespace ZCE_LIB
{

//============================================================================================

/*!
@brief      TRIE TREE��������ͷ�������ڴ�Ŵ˴Σ�����������������
            ʹ�õ�NODE������FREE��NODE������
*/
struct _shm_trie_tree_head
{
    //�Ҳ��ܶ�ģ������Ԫ�����ˣ���ʼ���ų�����
public:

    ///�ڴ����ĳ���
    size_t         size_of_mmap_ = 0;
    ///NODE������
    size_t         num_of_node_ = 0;

    ///FREE��NODE����
    size_t         size_free_node_ = 0;
    ///USE��NODE����
    size_t         size_use_node_ = 0;

    //���е�NODE�б��ͷָ��
    int32_t        freenode_list_next_ = -1;
};

//============================================================================================

///TRIE TREE������NODE������
template <class _letter_type> class _shm_trie_tree_node
{
public:

    ///
    _letter_type  letter_;
    ///�ֽڵ�
    int32_t    idx_brother_ = (-1);
    ///�ӽڵ�
    int32_t    idx_children_ = (-1);
    ///
    int32_t    idx_father_ = (-1);
};


//============================================================================================
/*!
@brief      �ֵ���(ORǰ׺��)�������ڹ����ڴ棨��ͨ�ڴ�Ҳ�У���һ���ڴ��У�ʹ�ã�
            Ҳ���Զ���̹�������Ȼͬ����������飬���Լ����ǣ�

            ������Ҫ�ĵ�ַ�ռ��С˵����ÿ��node������Ҫ2��shm_index_t��С�Ŀռ䣬

@tparam     _meta_type Ԫ���ͣ�ÿ��NODE����������
*/
template <class _letter_type,
    class _equal_letter = std::equal_to<_letter_type> > class smem_trie_tree
{
protected:

    //index��Ҫ������������,һ����ʾroot,
    static const int32_t ADDED_NUM_OF_NODE = 1;

    //index 1Ϊ ROOT
    static const int32_t ROOT_NODE_INDEX = 0;

    //�����ָ���ʾ,32λΪ0xFFFFFFFF,
    static const int32_t  _INVALID_NODE_INDEX = -1;

    typedef _shm_trie_tree_node<_letter_type> shm_trie_tree_node;
protected:

    //���е�ָ�붼�Ǹ��ӻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���
    //�ڴ������ַ
    char                        *smem_base_;

    //LIST��ͷ����ָ��
    _shm_trie_tree_head         *trietree_head_;

    //��������ʼָ��,
    shm_trie_tree_node          *node_base_;

    //���ڵ�ָ��,��1������λ��ʾ
    shm_trie_tree_node          *root_node_;

protected:

    ///����ڹ����ڴ�ʹ��,û��new,����ͳһ��initialize ��ʼ��
    ///�������,��������,���ǲ�������
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

    //ֻ����,��ʵ��,
    const smem_trie_tree<_letter_type, _equal_letter> & operator=(
        const smem_trie_tree<_letter_type, _equal_letter> &others) = delete;

protected:

    //����һ��NODE,�����FREELIST��ȡ��
    int32_t create_node(const _letter_type &val)
    {
        //���û�пռ���Է���
        if (trietree_head_->size_free_node_ == 0)
        {
            return _INVALID_NODE_INDEX;
        }

        //������ȡ1������
        int32_t free_idx = trietree_head_->freenode_list_next_;

        //��FREE������ժ����
        trietree_head_->freenode_list_next_ = (root_node_ + free_idx)->idx_brother_;
        //����ɾ�
        (node_base_ + free_idx)->idx_brother_ = _INVALID_NODE_INDEX;
        (node_base_ + free_idx)->idx_children_ = _INVALID_NODE_INDEX;
        (node_base_ + free_idx)->idx_father_ = _INVALID_NODE_INDEX;

        //��placement new��������,���Ҫ��Ҫ�����Ͻ�һ�㡣
        new ( &((node_base_ + free_idx)->letter_)) _letter_type(val) ;

        trietree_head_->size_use_node_  ++;
        trietree_head_->size_free_node_ --;

        return free_idx;
    }
    //�ͷ�һ��NODE,����黹��FREELIST
    void destroy_node(int32_t pos)
    {
        size_t free_next = trietree_head_->freenode_list_next_;

        (root_node_ + pos)->idx_brother_ = free_next;
        (node_base_ + pos)->idx_children_ = _INVALID_NODE_INDEX;
        (node_base_ + pos)->idx_father_ = _INVALID_NODE_INDEX;
        trietree_head_->freenode_list_next_ = pos;

        //������ʽ����������
        (node_base_ + pos)->~_letter_type();

        trietree_head_->size_use_node_  --;
        trietree_head_->size_free_node_ ++;

    }

public:

    //�ڴ����Ĺ���Ϊ ͷ����,NODE������������Ҫ�ĳ���,
    static std::size_t alloc_size(const int32_t num_node)
    {
        //NODE������Ϊ N+1��1Ϊroot
        return  sizeof(_shm_trie_tree_head)  + 
            sizeof(_shm_trie_tree_node<_letter_type> ) * (num_node + ADDED_NUM_OF_NODE) ;
    }

    smem_trie_tree<_letter_type, _equal_letter>* getinstance()
    {
        return this;
    }

    //��ʼ��
    static smem_trie_tree<_letter_type, _equal_letter>* initialize(const int32_t num_node, char *pmmap, bool if_restore = false)
    {
        assert(pmmap!=NULL && num_node >0 );
        _shm_trie_tree_head *treehead = reinterpret_cast<_shm_trie_tree_head *>(pmmap);

        //����ǻָ�,���ݶ����ڴ���,
        if (if_restore == true)
        {
            //���һ�»ָ����ڴ��Ƿ���ȷ,
            if (alloc_size(num_node) != treehead->size_of_mmap_ ||
                num_node != treehead->num_of_node_ )
            {
                return NULL;
            }
        }

        //��ʼ���ߴ�
        treehead->size_of_mmap_ = alloc_size(num_node);
        treehead->num_of_node_ = num_node;

        smem_trie_tree<_letter_type, _equal_letter>* instance = new smem_trie_tree<_letter_type>();

        //���е�ָ�붼�Ǹ��ӻ���ַ����õ���,���ڷ������,ÿ�γ�ʼ�������¼���
        instance->smem_base_ = pmmap;
        instance->trietree_head_ = treehead;
        instance->node_base_ = reinterpret_cast<shm_trie_tree_node *>(pmmap + sizeof(_shm_trie_tree_head));
        instance->root_node_ = instance->node_base_;
        //
        if (if_restore == false)
        {
            //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
            instance->clear();
        }

        assert(treehead->size_use_node_ + treehead->size_free_node_ == treehead->num_of_node_);

        //�����չ�
        return instance;
    }

    //�����ʼ�����е��ڴ�,���еĽڵ�ΪFREE
    void clear()
    {
        //����2���ؼ�Node,�Լ���س���,��ʼ���е�������free.
        trietree_head_->size_free_node_ = trietree_head_->num_of_node_;
        trietree_head_->size_use_node_ = 0;

        root_node_->idx_brother_ = _INVALID_NODE_INDEX;
        root_node_->idx_children_ = _INVALID_NODE_INDEX;
        root_node_->idx_father_ = _INVALID_NODE_INDEX;

        //��һ����root���ڶ�������FREE��
        trietree_head_->freenode_list_next_ = ADDED_NUM_OF_NODE;
        //��ʼ��free����������idx_brother_������
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
    //�ڲ�������ǰ����,����������
    bool full()
    {
        if (trietree_head_->size_free_node_ == 0 )
        {
            return true;
        }
        return false;
    };

    //�󽻼�
    bool has_word(const _letter_type *ward, size_t word_len, size_t *matching_len)
    {

        int32_t find_note = root_node_->idx_children_;

        _equal_letter   equal_meta;
        size_t n = 0;
        while (_INVALID_NODE_INDEX != find_note && n < word_len)
        {
            //�ҵ�һ��ƥ��ģ�Ѱ����һ��
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
        //����ƥ�䳤��
        *matching_len = n;
        //��ȫƥ��
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
                //û�ռ��ˡ�
                if (new_node == _INVALID_NODE_INDEX)
                {
                    return false;
                }
                //����
                //������һ����ѯ�����ݾ�������ҽ�
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
                //find_note Ӧ�þ���_INVALID_NODE_INDEX
                assert(find_note == _INVALID_NODE_INDEX);
            }
            else
            {
                //�ҵ�һ��ƥ��ģ�Ѱ����һ��
                bool if_find = false;
                //�Ȱ��ֵܶ����꣬���Ƿ���ƥ���
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

    //���������Ƿ���һ������
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

            //�ҵ�һ��ƥ��ģ�Ѱ����һ��
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

    //�����������Ѿ��е�Ԫ�ظ���
    size_t size()
    {
        return trietree_head_->size_use_node_;
    }
    //����������ӵ�����
    size_t capacity()
    {
        return trietree_head_->num_of_node_;
    }
    //
    size_t free()
    {
        return trietree_head_->size_free_node_;
    }

    //�ݹ�����е����ݶ�dump������������������
    void dump_tree()
    {
        dump_node(ROOT_NODE_INDEX);
    }

    void dump_node(int32_t index)
    {
        std::cout << index << ":[" << node_base_[index].idx_brother_ << "," 
            << node_base_[index].idx_children_ << ","
            << node_base_[index].idx_father_ << ",";
        //��ʽ����ע�͵������
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

