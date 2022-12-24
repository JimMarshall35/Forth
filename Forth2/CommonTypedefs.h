#ifndef COMMONTYPEDEFS
#define COMMONTYPEDEFS
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	False,
	True
}Bool;

// my solution to unintentional break case fallthrough in C / C++ -
// it might offend the sensibilities of some of you macro-hating "squares" - but for me it's worth it to know
// I'll never forget the break. You don't have to use it but you'd better get used to seeing it because this macro is the future 
// ;)
#define BCase break; case 


#ifdef __cplusplus
} // closing brace for extern "C"
#endif
#endif