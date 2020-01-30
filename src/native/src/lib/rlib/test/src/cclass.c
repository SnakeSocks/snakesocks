#include <rlib/c-with-class.h>
#include <stdio.h>
#include <stdlib.h>

typedef int element_type;

RCPP_CLASS_DECL(vector)
RCPP_CLASS_METHOD_EXTERN_DECL(vector, push_back, void, element_type)
RCPP_CLASS_METHOD_EXTERN_DECL(vector, at, element_type, int)

RCPP_CLASS_BEGIN(vector)
RCPP_CLASS_METHOD_DECL(vector, push_back, void, element_type)
RCPP_CLASS_METHOD_DECL(vector, at, element_type, int)
RCPP_CLASS_MEMBER_DECL(element_type *, data)
RCPP_CLASS_MEMBER_DECL(int, m_size)
RCPP_CLASS_MEMBER_DECL(int, m_cap)
RCPP_CLASS_END()

RCPP_CLASS_METHOD_IMPL(vector, push_back, void, element_type data) {
    if(this->m_size == this->m_cap) {
        this->m_cap += 1;
        this->m_cap *= 2;
        this->data = realloc(this->data, this->m_cap * sizeof(element_type));
    }
    this->data[this->m_size] = data;
    ++this->m_size;
}
RCPP_CLASS_METHOD_IMPL(vector, at, element_type, int index) {
    if(index >= this->m_size) {
        abort();
    }
    return this->data[index];
}

RCPP_CLASS_CONSTRUCTOR_IMPL(vector) {
    RCPP_CLASS_METHOD_REGISTER(vector, push_back)
    RCPP_CLASS_METHOD_REGISTER(vector, at)
    this->m_cap = this->m_size = 0;
    this->data = NULL;
    printf("Constructor called!\n");
}
RCPP_CLASS_DESTRUCTOR_IMPL(vector) {
    if(this->data)
        free(this->data);
    printf("Destructor called!\n");
}

int main(){
    RCPP_NEW(vector, vct, NULL);
    RCPP_CALL(vct, push_back, 3);
    RCPP_CALL(vct, push_back, 2);
    RCPP_CALL(vct, push_back, 1);
    vct.push_back(&vct, 6);
    vct.push_back(&vct, 7);
    vct.push_back(&vct, 8);

    for(int cter = 0; cter < vct.m_size; ++cter) {
        printf("Element at index %d is %d.\n", cter, RCPP_CALL(vct, at, cter));
    }
    return 0;
}
