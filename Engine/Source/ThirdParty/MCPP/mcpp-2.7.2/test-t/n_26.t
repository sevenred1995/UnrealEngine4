/* n_26.t:  The name once replaced is not furthur replaced. */

/* 26.1:    Directly recursive object-like macro definition.    */
/*  Z[0];   */
#define Z   Z[0]
    Z;

/* 26.2:    Intermediately recursive object-like macro definition.  */
/*  AB; */
#define AB  BA
#define BA  AB
    AB;

/* 26.3:    Directly recursive function-like macro definition.  */
/*  x + f(x);   */
#define f(a)    a + f(a)
    f( x);

/* 26.4:    Intermediately recursive function-like macro definition.    */
/*  x + x + g( x);  */
#define g(a)    a + h( a)
#define h(a)    a + g( a)
    g( x);

/* 26.5:    Rescanning encounters the non-replaced macro name.  */
/*  Z[0] + f( Z[0]);    */
    f( Z);

