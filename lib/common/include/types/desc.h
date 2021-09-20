#ifndef __COMMON_TYPES_DESC_H__
#define __COMMON_TYPES_DESC_H__

#define DECL_TYPE_DESC(type) bool (*copy) (type* dest, const type* src);\
    void (*move)(type* dest, type* src);\
    bool (*eq)(const type* e1, const type* e2);\
    void (*destruct)(type* this);\
    const size_t size;

#define DEF_TYPE_DESC(copy, move, eq, del, size) copy, move, eq, del, size

typedef struct {
    bool (*copy)(void* dest, const void* src);
    void (*move)(void* dest, void* src);
    bool (*eq)(const void* e1, const void* e2);
    void (*destruct)(void* self);
    const size_t size;
} type_desc_t;

bool type_is_deletable(const type_desc_t* desc)
{
    return desc->destruct != 0;
}

bool type_is_movable(const type_desc_t* desc)
{
    return desc->move != 0;
}

bool type_is_copiable(const type_desc_t* desc)
{
    return desc->copy != 0;
}

bool type_eq(const type_desc_t* desc, void* self, void* other)
{
    return desc->eq(self, other);
}

void type_move(const type_desc_t* desc, void* dest, void* src)
{
    desc->move(dest, src);
}

bool type_copy(const type_desc_t* desc, void* dest, const void* src)
{
    return desc->copy(dest, src);
}

void type_destruct(const type_desc_t* desc, void* element)
{
    if(type_is_deletable(desc))
        desc->destruct(element);
}

#endif