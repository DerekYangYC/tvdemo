#pragma once

template <typename Object>
class object_pool;

class object_pool_access
{
public:
  template <typename Object>
  static Object* create()
  {
    return new Object;
  }

  template <typename Object>
  static void destroy(Object* o)
  {
    delete o;
  }

  template <typename Object>
  static Object*& next(Object* o)
  {
    return o->next_;
  }

  template <typename Object>
  static Object*& prev(Object* o)
  {
    return o->prev_;
  }
};

template <typename Object>
class object_pool
{
public:
  // Constructor.
  object_pool()
    : live_list_(0),
      free_list_(0)
  {
  }

  // Destructor destroys all objects.
  ~object_pool()
  {
    destroy_list(live_list_);
    destroy_list(free_list_);
  }

  // Get the object at the start of the live list.
  Object* first()
  {
    return live_list_;
  }

  // Allocate a new object.
  Object* alloc()
  {
    //MutexLockGuard lock(mutex_);
    Object* o = free_list_;
    if (o)
      free_list_ = object_pool_access::next(free_list_);
    else
      o = object_pool_access::create<Object>();

    object_pool_access::next(o) = live_list_;
    object_pool_access::prev(o) = 0;
    if (live_list_)
      object_pool_access::prev(live_list_) = o;
    live_list_ = o;

    return o;
  }

  // Free an object. Moves it to the free list. No destructors are run.
  void free(Object* o)
  {
   // MutexLockGuard lock(mutex_);
    if (live_list_ == o)
      live_list_ = object_pool_access::next(o);

    if (object_pool_access::prev(o))
    {
      object_pool_access::next(object_pool_access::prev(o))
        = object_pool_access::next(o);
    }

    if (object_pool_access::next(o))
    {
      object_pool_access::prev(object_pool_access::next(o))
        = object_pool_access::prev(o);
    }

    object_pool_access::next(o) = free_list_;
    object_pool_access::prev(o) = 0;
    free_list_ = o;
  }

private:
  // Helper function to destroy all elements in a list.
  void destroy_list(Object* list)
  {
    while (list)
    {
      Object* o = list;
      list = object_pool_access::next(o);
      object_pool_access::destroy(o);
    }
  }

  // The list of live objects.
  Object* live_list_;

  // The free list.
  Object* free_list_;

 //MutexLock mutex_;
};

struct Allocator
{
    virtual char* alloc() = 0;
    virtual void free(char* pData, size_t size) = 0;

    size_t chunkSize_;
};

template<int n>
class TAllocator : public Allocator
{
public:
    TAllocator()
    {
        chunkSize_ = n;
    }

    struct DataChunk
    {
        char data_[n];
        DataChunk* next_;
        DataChunk* prev_;
    };

    char* alloc()
    {
        return reinterpret_cast<char*>(pool_.alloc());
    }

    void free(char* pData, size_t size)
    {
        //assert(size != sizeof(DataChunk));
        pool_.free(reinterpret_cast<DataChunk*>(pData));
    }

private:
    object_pool<DataChunk> pool_;
};

#define ALLOC_NUM 16
class ObjectAllocator
{
public:
    ObjectAllocator()
    {
        pAllocator_[0] = new TAllocator<32>();
        pAllocator_[1] = new TAllocator<64>();
        pAllocator_[2] = new TAllocator<128>();
        pAllocator_[3] = new TAllocator<256>();
        pAllocator_[4] = new TAllocator<512>();
        pAllocator_[5] = new TAllocator<1024>();
        pAllocator_[6] = new TAllocator<2048>();
        pAllocator_[7] = new TAllocator<4096>();
        pAllocator_[8] = new TAllocator<8192>();
        pAllocator_[9] = new TAllocator<1024 * 32>();
        pAllocator_[10] = new TAllocator<1024 * 64>();
        pAllocator_[11] = new TAllocator<1024 * 128>();
        pAllocator_[12] = new TAllocator<1024 * 256>();
        pAllocator_[13] = new TAllocator<1024 * 512>();
        pAllocator_[14] = new TAllocator<1024 * 1024>();
        pAllocator_[15] = new TAllocator<1024 * 2048>();
    }

    char* alloc(size_t size)
    {
        for(int i = 0; i < ALLOC_NUM; ++i)
        {
            if(size <= pAllocator_[i]->chunkSize_)
            {
                return pAllocator_[i]->alloc();
             }
        }

        return new char[size];
    }

    void free(char* pData, size_t size)
    {
        for(int i = 0; i < ALLOC_NUM; ++i)
        {
            if(size <= pAllocator_[i]->chunkSize_)
            {
                pAllocator_[i]->free(pData, size);
                return;
            }
        }

		delete [] pData;
    }

private:
    Allocator* pAllocator_[ALLOC_NUM];
};

