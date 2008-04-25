
//sh4 core il interface

//load a constant value to an iconst
struct iconst { u32 val; };	//any value
iconst loadci(u32 value);

//load a constant value to an fconst
struct fconst { u32 idx; };	//0 or 1
enum FLOAT_CONSTNATS { FC_ZERO ,FC_ONE };
fconst loadcf(FLOAT_CONSTNATS fc);

//registers
struct freg { u32 idx; };	//fpu reg
struct ireg { u32 idx; };	//integer reg
struct breg { u32 idx; };	//boolean reg -- can be true or false
//registers can be read by loadrf/loadri and writen with storerf/storeri.
//Temporaries can be created with alloctf/allocti and must be freed with freetf/freeti



