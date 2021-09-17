#ifndef __COMMON_MACRO_VECTOR_H__
#define __COMMON_MACRO_VECTOR_H__

#define VECTOR_DECL(type_t, typealias)\
typedef struct typealias ## _vector_t typealias ## _vector_t;\
typedef struct typealias ## _vector_iterator_t typealias ## _vector_iterator_t;\
\
struct typealias ## _vector_t {\
    vector_t impl;\
};\
\
const typealias ## _vector_t typealias ## _vector_init = {vector_init};\
\
struct typealias ## _vector_iterator_t {\
    DECL_ITERATOR(struct typealias ## _vector_iterator_t, type_t);\
    vector_iterator_t impl;\
};\
\
typealias ## _vector_t typealias ## _vector(size_t capacity, allocator_t* elements_allocator);\
bool typealias ## _vector_create(typealias ## _vector_t* vec, size_t capacity, allocator_t* allocator);\
void typealias ## _vector_delete(typealias ## _vector_t* vec);\
bool typealias ## _vector_get(typealias ## _vector_t* vec, type_t** out, unsigned int index);\
void typealias ## _vector_move(typealias ## _vector_t* dest, typealias ## _vector_t* src);\
bool typealias ## _vector_copy(typealias ## _vector_t* dest, const typealias ## _vector_t* src, allocator_t* allocator);\
bool typealias ## _vector_eq(const typealias ## _vector_t* v1, const typealias ## _vector_t* v2);\
bool typealias ## _vector_move_add(typealias ## _vector_t* vec, type_t* element);\
bool typealias ## _vector_copy_add(typealias ## _vector_t* vec, const type_t* element, allocator_t* allocator);\
void typealias ## _vector_iter(typealias ## _vector_t* vec, typealias ## _vector_iterator_t* it);\
\
bool typealias ## _vector_iterator_next(typealias ## _vector_iterator_t* it);\
type_t* typealias ## _vector_iterator_get(typealias ## _vector_iterator_t* it);\
\
const typealias ## _vector_iterator_t typealias ## _vector_iterator_init = {typealias ## _vector_iterator_next, typealias ## _vector_iterator_get, vector_it_init};\
\
typealias ## _vector_t typealias ## _vector(size_t capacity, allocator_t* elements_allocator)\
{\
    typealias ## _vector_t tmp;\
    typealias ## _vector_create(&tmp, capacity, elements_allocator);\
    return tmp;\
}\
bool typealias ## _vector_create(typealias ## _vector_t* vec, size_t capacity, allocator_t* elements_allocator)\
{\
    return vector_create(\
        &vec->impl, \
        (type_desc_t*)(&typealias ## _desc), \
        capacity, \
        elements_allocator\
    );\
}\
\
void typealias ## _vector_delete(typealias ## _vector_t* vec)\
{\
    vector_delete(&vec->impl);\
}\
\
void typealias ## _vector_move(typealias ## _vector_t* dest, typealias ## _vector_t* src)\
{\
    vector_move(&dest->impl, &src->impl);\
}\
\
bool typealias ## _vector_copy(typealias ## _vector_t* dest, const typealias ## _vector_t* src, allocator_t* allocator)\
{\
    return vector_copy(&dest->impl, &src->impl, allocator);\
}\
\
bool typealias ## _vector_get(typealias ## _vector_t* vec, type_t** out, unsigned int index)\
{\
    return vector_get(&vec->impl, (void**) out, index);\
}\
\
bool typealias ## _vector_eq(const typealias ## _vector_t* v1, const typealias ## _vector_t* v2)\
{\
    return vector_eq(&v1->impl, &v2->impl);\
}\
\
bool typealias ## _vector_move_add(typealias ## _vector_t* vec, type_t* element)\
{\
    return vector_move_add(&vec->impl, element);\
}\
\
bool typealias ## _vector_copy_add(typealias ## _vector_t* vec, const type_t* element, allocator_t* allocator)\
{\
    return vector_copy_add(&vec->impl, element, allocator);\
}\
\
void typealias ## _vector_iter(typealias ## _vector_t* vec, typealias ## _vector_iterator_t* it)\
{\
    vector_iter(&vec->impl, &it->impl);\
    it->next = typealias ## _vector_iterator_next;\
    it->get = typealias ## _vector_iterator_get;\
}\
\
bool typealias ## _vector_iterator_next(typealias ## _vector_iterator_t* it)\
{\
    return vector_iterator_next(&it->impl);\
}\
\
type_t* typealias ## _vector_iterator_get(typealias ## _vector_iterator_t* it)\
{\
    return (type_t*) vector_iterator_get(&it->impl);\
}\

#endif