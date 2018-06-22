/*!
 * \file mchatv1_macro_hell.h
 * \author Sean Tracy
 * \date 8 March 2017
 * \version 0.0.1
 * \brief Some crazy macros for crazy things.
 *
 * \details
 * These are some crazy macros to do some insane preprocessor things.
 * You are not expected to understand these, I know I don't.  The only
 * one I wrote is COUNT(), and it's just a cheap rip-off of FOR_EACH().
 *
 * Based on the work Saad Ahmad (Who based his work on the work of
 * Paul Fultz II in his cloak library).
 *
 * \sa http://saadahmad.ca/cc-preprocessor-metaprogramming-2/
 * \sa https://github.com/pfultz2/Cloak
 *
 * \todo Document these individually (eventually)
 */
#ifndef MCHATV1_MACRO_HELL_H
#define MCHATV1_MACRO_HELL_H

//! \cond FOO

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define CAT(x, y) x ## y
#define IF_1(t, ...) t
#define IF_0(t, ...) __VA_ARGS__
#define IF(value) CAT(IF_,value)

#define EAT(...)
#define EXPAND_TEST_EXISTS(...) EXPANDED, EXISTS(__VA_ARGS__) ) EAT (
#define GET_TEST_EXISTS_RESULT(x) ( CAT(EXPAND_TEST_, x), DOESNT_EXIST )
#define GET_TEST_EXIST_VALUE_(expansion, existValue) existValue
#define GET_TEST_EXIST_VALUE(x) GET_TEST_EXIST_VALUE_ x
#define TEST_EXISTS(x) GET_TEST_EXIST_VALUE ( GET_TEST_EXISTS_RESULT(x) )

#define DOES_VALUE_EXIST_EXISTS(...) 1
#define DOES_VALUE_EXIST_DOESNT_EXIST 0
#define DOES_VALUE_EXIST(x) CAT(DOES_VALUE_EXIST_, x)
#define EXTRACT_VALUE_EXISTS(...) __VA_ARGS__
#define EXTRACT_VALUE(value) CAT(EXTRACT_VALUE_, value)
#define TRY_EXTRACT_EXISTS(value, ...) \
    IF ( DOES_VALUE_EXIST(TEST_EXISTS(value)) ) \
    ( EXTRACT_VALUE(value), __VA_ARGS__ )

#define NOT_0 EXISTS(1)
#define NOT(x) TRY_EXTRACT_EXISTS( CAT(NOT_, x), 0 )
#define HEAD(x, ...) x
#define TAIL(x, ...) __VA_ARGS__

#define EMPTY()
#define DEFER(id) id EMPTY()
#define DEFER2(...) __VA_ARGS__ DEFER(EMPTY) ()
#define DEFER3(...) __VA_ARGS__ DEFER2(EMPTY) ()
#define DEFER4(...) __VA_ARGS__ DEFER3(EMPTY) ()
#define DEFER5(...) __VA_ARGS__ DEFER4(EMPTY) ()
#define DEFER6(...) __VA_ARGS__ DEFER5(EMPTY) ()
#define DEFER7(...) __VA_ARGS__ DEFER6(EMPTY) ()
#define DEFER8(...) __VA_ARGS__ DEFER7(EMPTY) ()

#define EVAL_1(...) __VA_ARGS__
#define EVAL_2(...) EVAL_1(EVAL_1(__VA_ARGS__))
#define EVAL_3(...) EVAL_2(EVAL_2(__VA_ARGS__))
#define EVAL_4(...) EVAL_3(EVAL_3(__VA_ARGS__))
#define EVAL_5(...) EVAL_4(EVAL_4(__VA_ARGS__))
#define EVAL_6(...) EVAL_5(EVAL_5(__VA_ARGS__))
#define EVAL_7(...) EVAL_6(EVAL_6(__VA_ARGS__))
#define EVAL_8(...) EVAL_7(EVAL_7(__VA_ARGS__))
#define EVAL(...) EVAL_8(__VA_ARGS__)


#define TEST_LAST EXISTS(1)
#define IS_LIST_EMPTY(...) \
    TRY_EXTRACT_EXISTS( \
        DEFER(HEAD) (__VA_ARGS__ EXISTS(1)) \
        , 0)

#define IS_LIST_NOT_EMPTY(...) NOT(IS_LIST_EMPTY(__VA_ARGS__))
#define FOR_EACH_INDIRECT() FOR_EACH_NO_EVAL
#define FOR_EACH_NO_EVAL(f, ...) \
    IF ( IS_LIST_NOT_EMPTY( __VA_ARGS__ ) ) \
    ( f(HEAD(__VA_ARGS__)) \
      DEFER2(FOR_EACH_INDIRECT) () (f, TAIL(__VA_ARGS__)))

#define FOR_EACH(f, ...) \
    EVAL(FOR_EACH_NO_EVAL(f, __VA_ARGS__))

#define COUNT_INDIRECT() COUNT_NO_EVAL
#define COUNT_NO_EVAL(...) \
    IF ( IS_LIST_NOT_EMPTY( __VA_ARGS__) ) \
    (  1 + DEFER2(COUNT_INDIRECT) () (TAIL(__VA_ARGS__)), 0)

#define COUNT(...) \
    EVAL(COUNT_NO_EVAL(__VA_ARGS__))

#define FOR_EACH_INDEX_INDIRECT() FOR_EACH_INDEX_NO_EVAL
#define FOR_EACH_INDEX_NO_EVAL(f, i, ...) \
    IF ( IS_LIST_NOT_EMPTY( __VA_ARGS__) ) \
    ( f(HEAD(__VA_ARGS__), i ) \
    DEFER2(FOR_EACH_INDEX_INDIRECT) () (f, i + 1, TAIL(__VA_ARGS__)))

#define FOR_EACH_INDEX(f, i, ...) \
    EVAL(FOR_EACH_INDEX_NO_EVAL(f, i, __VA_ARGS__))

// Generalized Map Macros (X Macros)
//! Make a list of each of the first argument for each X Macro call
#define MAP_MACRO_MAKE_LIST(name, ...) name,

//! Count the number of items in the Map Macro (X Macro)
#define MAP_MACRO_MAPPED_COUNT(...) + 1
#define MAPPED_COUNT(MAP) (0 MAP(MAP_MACRO_MAPPED_COUNT))

//! Macro to create error string arrays
#define MAP_MACRO_ERROR_STRING_ARRAY(error_name, error_string) error_string,
#define ERROR_STRING_ARRAY(MAP) MAP(MAP_MACRO_ERROR_STRING_ARRAY)

#endif // DOXYGEN_SHOULD_SKIP_THIS
//! \endcond

#endif // MCHATV1_MACRO_HELL_H
