//sh4 core native gen interface

//load a constant value to an iconst
struct ng_iconst { u32 val; };	//any value
iconst ng_loadci(u32 value);

const ng_iconst ng_0i = {0};

//load a constant value to an fconst
struct ng_fconst { u32 idx; };	//0 or 1
enum ng_FLOAT_CONSTNATS { FC_ZERO ,FC_ONE };
fconst ng_loadcf(ng_FLOAT_CONSTNATS fc);

//registers
struct ng_freg { u32 idx; };	//fpu reg
struct ng_ireg { u32 idx; };	//integer reg
//struct ng_breg { u32 idx; };	//boolean reg -- can be true or false
typedef ng_ireg ng_breg;
//registers can be read by loadrf/loadri and writen with storerf/storeri.
//Temporaries can be created with alloctf/allocti and must be freed with freetf/freeti
ng_ireg ng_allocti(u32 shreg);

void ng_add(ng_ireg dst,ng_ireg r1,ng_ireg r2);
void ng_addc(ng_ireg dst,ng_breg co,ng_ireg r1,ng_ireg r2,ng_breg cin);
void ng_sub(ng_ireg dst,ng_ireg r1,ng_ireg r2);

void ng_readmem(ng_ireg dst,ng_ireg base,ng_iconst offs);
void ng_readmem(ng_freg dst,ng_ireg base,ng_iconst offs);
void ng_writemem(ng_ireg src,ng_ireg base,ng_iconst offs);
void ng_writemem(ng_freg src,ng_ireg base,ng_iconst offs);


