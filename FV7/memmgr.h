#pragma once

template <class T>
class CMemoryBase
{
public:
    void* p;
    CMemoryBase() : p(NULL) {}
    CMemoryBase(void* lp) throw() { p = lp; }
    ~CMemoryBase() { Free(); }
    void* Alloc(size_t size) {
        Free();
        T* pT = static_cast<T*>(this);
        p = pT->allocate(size);
        return p;
    }
    void Free()
    {
        if (p) {
            T* pT = static_cast<T*>(this);
            pT->deallocate(p);
            p = NULL;
        }
    }
    virtual void* allocate(size_t size) { return NULL; }
    virtual void deallocate(void* pv) { }
    operator void* () const throw() { return p; }
    void** operator&() throw() { return &p; }
    bool operator!() const throw() { return (p == NULL); }
    operator PWSTR() const throw() { return (PWSTR)p; }
    operator PCWSTR() const throw() { return (PCWSTR)p; }
    void operator=(void* srcp) throw() { Attach(srcp); }

    void Attach(void* p2) throw()
    {
        Free();
        p = p2;
    }
    // Detach the interface (does not Release)
    void* Detach() throw()
    {
        void* pt = p;
        p = NULL;
        return pt;
    }
};

class CMemStd : public CMemoryBase<CMemStd> {
public:
    CMemStd() {}
    CMemStd(void* lp) : CMemoryBase<CMemStd>(lp) {}
    void* allocate(size_t size) { return malloc(size); }
    void deallocate(void* pv) { free(pv); }
};

class CMemTask : public CMemoryBase<CMemTask> {
public:
    CMemTask() {}
    CMemTask(void* lp) : CMemoryBase<CMemTask>(lp) {}
    void* allocate(size_t size) { return CoTaskMemAlloc(size); }
    void deallocate(void* pv) { CoTaskMemFree(pv); }
};

class CMemLocal : public CMemoryBase<CMemLocal> {
public:
    CMemLocal() {}
    CMemLocal(void* lp) : CMemoryBase<CMemLocal>(lp) {}
    void* allocate(size_t size) { return LocalAlloc(LPTR, size); }
    void deallocate(void* pv) { LocalFree(pv); }
};
