#include "utils.h"

pairing_t * 
load_pairing(char *params_path) {
  char param[2048];
  pairing_t *pairing = malloc(sizeof(pairing_t));

  FILE * file = fopen(params_path, "r");
  size_t read = fread(param, 1, 2048, file);
  fclose(file);

  if (!read) pbc_die("Error reading the parameter file");
  pairing_init_set_buf(*pairing, param, read);
  return pairing;
}


/* transpose an nxn matrix */
element_t ** 
transpose(element_t **m, int n) {

    element_t t;
    element_init_same_as(t, m[0][0]);

    for (int i=0;i<(n-1);++i) {
        for (int j=i+1;j<n;++j) {
            element_set(t,m[i][j]);
            element_set(m[i][j],m[j][i]);
            element_set(m[j][i],t);
        }
    }
}

/* sets res=a*b-c*d */
void 
minorX(element_t * res, element_t a, element_t b, element_t c, element_t d){

    element_t tmp;

    element_init_same_as(*res,a);
    element_init_same_as(tmp,a);
    element_mul(*res,a,b);
    element_mul(tmp,c,d);
    element_sub(*res,*res,tmp);

    element_clear(tmp);

}


/* multiplies 
    v: a 3-vector of elements in Zr by 
    m: a 3x3 matrix of elements of G1 / G2
    r: is the resulting  3-vector of elements of G1/G2
    r is assumed allocated
*/

void
v_times_m(element_t *r, element_t *v, element_t **m) 
{

  element_t t;

  element_init_same_as(t, m[0][0]);  

  element_mul_zn(r[0],m[0][0],v[0]); 
     element_mul_zn(t,m[1][0],v[1]); element_add(r[0],r[0],t);
     element_mul_zn(t,m[2][0],v[2]); element_add(r[0],r[0],t);

  element_mul_zn(r[1],m[0][1],v[0]); 
     element_mul_zn(t,m[1][1],v[1]); element_add(r[1],r[1],t);
     element_mul_zn(t,m[2][1],v[2]); element_add(r[1],r[1],t);
  

  element_mul_zn(r[2],m[0][2],v[0]); 
     element_mul_zn(t,m[1][2],v[1]); element_add(r[2],r[2],t);
     element_mul_zn(t,m[2][2],v[2]); element_add(r[2],r[2],t);

}

void
printInfo(pairing_t pairing)
{
    if (pairing_is_symmetric(pairing))
        printf("Symmetric\n");
    else
        printf("Asymmetric\n");

    printf("Elements of G1 are %d bytes long (compressed: %d)\n",pairing_length_in_bytes_G1(pairing),pairing_length_in_bytes_compressed_G1(pairing));
    printf("Elements of G2 are %d bytes long (compressed: %d)\n",pairing_length_in_bytes_G2(pairing),pairing_length_in_bytes_compressed_G2(pairing));
    printf("Elements of GT are %d bytes long \n",pairing_length_in_bytes_GT(pairing));
    printf("Elements of Zr are %d bytes long \n",pairing_length_in_bytes_Zr(pairing));

}


