#ifndef RLIB_CWITHCLASS_H_
#define RLIB_CWITHCLASS_H_

//TODO: clean namespace.
//TODO: use macro to type class_name only once.
//#error c_with_class not completed yet

#ifdef __cplusplus
#error You should not use c-with-class.h in real C++.
#endif

#define RCPP_NEW(type,name,constructor_arg) struct type name __attribute__((cleanup(type##_rcpp_destructor)));type##_rcpp_constructor(&name,constructor_arg)
#define RCPP_CALL(i_objectname,i_funcname, ...) i_objectname.i_funcname(&i_objectname, ##__VA_ARGS__) //ONLY static public function can be called directly!!!
#define RCPP_PCALL(p_objectname,i_funcname, ...) p_objectname->i_funcname(p_objectname, ##__VA_ARGS__)

#define RCPP_CLASS_DECL(class_name) struct class_name;
#define RCPP_CLASS_METHOD_EXTERN_DECL(class_name, method_name, return_type, ...) typedef return_type (* class_name##method_name##_rcpp_t)(struct class_name *this, ##__VA_ARGS__); //VAARGS is `int arg1, float arg2, ...`
#define RCPP_CLASS_BEGIN(class_name) struct class_name {
#define RCPP_CLASS_METHOD_DECL(class_name, method_name, ...) RCPP_CLASS_MEMBER_DECL(class_name##method_name##_rcpp_t, method_name)
#define RCPP_CLASS_MEMBER_DECL(type, name) type name;
#define RCPP_CLASS_END() };
#define RCPP_CLASS_METHOD_IMPL(class_name, method_name, return_type, ...) return_type class_name##method_name##_rcpp_impl(struct class_name *this, ##__VA_ARGS__) //VAARGS is `int arg1, float arg2, ...`
#define RCPP_CLASS_CONSTRUCTOR_IMPL(class_name) void class_name##_rcpp_constructor(struct class_name *this, void *arg) //TODO: Register all methods.
#define RCPP_CLASS_METHOD_REGISTER(class_name, method_name) this->method_name = &class_name##method_name##_rcpp_impl;
#define RCPP_CLASS_DESTRUCTOR_IMPL(class_name) void class_name##_rcpp_destructor(struct class_name *this)

#endif
