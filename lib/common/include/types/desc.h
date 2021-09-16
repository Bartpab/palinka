#ifndef __COMMON_TYPES_DESC_H__
#define __COMMON_TYPES_DESC_H__

#define DECL_TYPE_DESC(type) bool (*copy) (type* dest, const type* src, allocator_t* allocator);\
    void (*move)(type* dest, type* src);\
    bool (*eq)(const type* e1, const type* e2);\
    void (*del)(type* this);\
    const size_t size;

#define DEF_TYPE_DESC(copy, move, eq, del, size) copy, move, eq, del, size

typedef struct {
    bool (*copy)(void* dest, const void* src, allocator_t* allocator);
    void (*move)(void* dest, void* src);
    bool (*eq)(const void* e1, const void* e2);
    void (*del)(void* this);
    const size_t size;
} type_desc_t;


#endif