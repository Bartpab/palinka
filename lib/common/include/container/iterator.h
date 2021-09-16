#ifndef __CONTAINER_IT_H__
#define __CONTAINER_IT_H__

#define DECL_ITERATOR(iterator_t, type_t) bool (*next)(iterator_t* it);\
    type_t* (*get)(iterator_t* it);


#endif