#include "hve_amm.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>


setup_t
setup(pairing_t *pairing, element_t psi, element_t g1, element_t g2, int l)
{
  element_t ***BB, ***CC;
  element_t **A1, **A2, **B, **C, **X, **Xs;
  element_t gT;
  element_t tmp1,tmp2;
  element_t ptmp;
  element_t t, t1,t2,t3;
  element_t delta;

  element_init_Zr(delta,*pairing);
  element_init_Zr(t1,*pairing);
  element_init_Zr(t2,*pairing);
  element_init_Zr(t3,*pairing);



  element_init_G1(tmp1,*pairing);
  element_init_G2(tmp2,*pairing);

  /* compute gT=e(g1,g2)^psi */
  element_init_GT(gT, *pairing);
  element_init_GT(t, *pairing);
  pairing_apply(t, g1, g2, *pairing);
  element_pow_zn(gT, t, psi);

  /* if you want to see psi, g1, g2, gT */
#ifdef VERBOSE
  printf("psi=\n");
  element_printf("%B\n",psi);
  element_printf("%B\n",g2);
  printf("gT=\n");
  element_printf("%B\n",gT);
#endif

  /* constructing the canonical bases: A1 for G1 and A2 for G2 */
  A1=malloc(sizeof(element_t *)*3);  A1[0]=malloc(sizeof(element_t)*3);
  A1[1]=malloc(sizeof(element_t)*3); A1[2]=malloc(sizeof(element_t)*3);

  A2=malloc(sizeof(element_t *)*3);  A2[0]=malloc(sizeof(element_t)*3);
  A2[1]=malloc(sizeof(element_t)*3); A2[2]=malloc(sizeof(element_t)*3);

  for(int i=0;i<3;i++){
    for(int j=0;j<3;j++){
        element_init_G1(A1[i][j],*pairing); element_init_G2(A2[i][j],*pairing);
        if (i==j){
    	    element_set(A1[i][j],g1);  /* element_set(dest,source) */
	        element_set(A2[i][j],g2);  /* element_set(dest,source) */
        }
        else{
	        element_set0(A1[i][j]);
	        element_set0(A2[i][j]);
        }
    }
  }

  BB=(element_t ***)malloc(sizeof(element_t **)*(l+1));
  CC=(element_t ***)malloc(sizeof(element_t **)*(l+1));

  setup_t setup = malloc(sizeof(setup_t));
  mpk_t public = setup->public = malloc(sizeof(*public));
  setup->public->B=BB;
  msk_t private = setup->private = malloc(sizeof(*private));
  setup->private->C=CC;

  element_init_GT(public->gT,*pairing);
  element_set(public->gT, gT);
  setup->private->l = setup->public->l = l;

  for(int zz=0;zz<l+1;zz++){
    BB[zz]=malloc(sizeof(element_t *)*3); CC[zz]=malloc(sizeof(element_t *)*3);
    BB[zz][0]=malloc(sizeof(element_t)*3); CC[zz][0]=malloc(sizeof(element_t)*3);
    BB[zz][1]=malloc(sizeof(element_t)*3); CC[zz][1]=malloc(sizeof(element_t)*3);
    BB[zz][2]=malloc(sizeof(element_t)*3); CC[zz][2]=malloc(sizeof(element_t)*3);

    B=BB[zz]; C=CC[zz];
    X=malloc(sizeof(element_t *)*3); Xs=malloc(sizeof(element_t *)*3);
    X[0]=malloc(sizeof(element_t)*3); Xs[0]=malloc(sizeof(element_t)*3);
    X[1]=malloc(sizeof(element_t)*3); Xs[1]=malloc(sizeof(element_t)*3);
    X[2]=malloc(sizeof(element_t)*3); Xs[2]=malloc(sizeof(element_t)*3);
    /* pick a random X */
    for(int i=0;i<3;i++){
      for(int j=0;j<3;j++){
        element_init_Zr(X[i][j],*pairing);
	    element_random(X[i][j]);
      }
    }

    /* let us invert X in Xs*/
    minorX(&ptmp, X[1][1],X[2][2],X[1][2],X[2][1]);
    element_init_Zr(Xs[0][0],*pairing);
    element_set(Xs[0][0],ptmp); element_clear(ptmp);
    minorX(&ptmp, X[1][2],X[2][0],X[1][0],X[2][2]);
    element_init_Zr(Xs[1][0],*pairing);
    element_set(Xs[1][0],ptmp); element_clear(ptmp);
    minorX(&ptmp, X[1][0],X[2][1],X[1][1],X[2][0]);
    element_init_Zr(Xs[2][0],*pairing);
    element_set(Xs[2][0],ptmp); element_clear(ptmp);

    minorX(&ptmp, X[0][2],X[2][1],X[0][1],X[2][2]);
    element_init_Zr(Xs[0][1],*pairing);
    element_set(Xs[0][1],ptmp); element_clear(ptmp);
    minorX(&ptmp, X[0][0],X[2][2],X[0][2],X[2][0]);
    element_init_Zr(Xs[1][1],*pairing);
    element_set(Xs[1][1],ptmp); element_clear(ptmp);
    minorX(&ptmp, X[0][1],X[2][0],X[0][0],X[2][1]);
    element_init_Zr(Xs[2][1],*pairing);
    element_set(Xs[2][1],ptmp); element_clear(ptmp);

    minorX(&ptmp, X[0][1],X[1][2],X[0][2],X[1][1]);
    element_init_Zr(Xs[0][2],*pairing);
    element_set(Xs[0][2],ptmp); element_clear(ptmp);
    minorX(&ptmp, X[0][2],X[1][0],X[0][0],X[1][2]);
    element_init_Zr(Xs[1][2],*pairing);
    element_set(Xs[1][2],ptmp); element_clear(ptmp);
    minorX(&ptmp, X[0][0],X[1][1],X[0][1],X[1][0]);
    element_init_Zr(Xs[2][2],*pairing);
    element_set(Xs[2][2],ptmp); element_clear(ptmp);

    /* computing the determinat of X */
    element_mul(t1,X[0][0],X[1][1]); element_mul(t1,t1,X[2][2]);
    element_mul(t2,X[0][1],X[1][2]); element_mul(t2,t2,X[2][0]);
    element_mul(t3,X[0][2],X[1][0]); element_mul(t3,t3,X[2][1]);
    element_add(delta,t1,t2); element_add(delta,delta,t3);
    element_mul(t1,X[0][2],X[1][1]); element_mul(t1,t1,X[2][0]);
    element_mul(t2,X[0][0],X[1][2]); element_mul(t2,t2,X[2][1]);
    element_mul(t3,X[0][1],X[1][0]); element_mul(t3,t3,X[2][2]);
    element_sub(delta,delta,t1);   element_sub(delta,delta,t2);
    element_sub(delta,delta,t3);
#ifdef VERBOSE
        element_printf("Determinante:%B\n",delta);
#endif

    for(int i=0;i<3;i++){
      for(int j=0;j<3;j++){
        element_div(Xs[i][j],Xs[i][j],delta);
        element_mul(Xs[i][j],Xs[i][j],psi);
      }
    }
    /* now X * Xs = psi * I */

    /* if you want to check X and Xs */
#ifdef VERBOSE
    for(int i=0; i<3; ++i){
      for(int j=0; j<3; ++j){
        element_mul(t1,X[i][0],Xs[0][j]);
        element_mul(t2,X[i][1],Xs[1][j]);
        element_mul(t3,X[i][2],Xs[2][j]);
        element_add(t1,t1,t2);
        element_add(t1,t1,t3);
        element_printf("(X*Xs)[%d][%d] %B\n",i,j,t1);
      }
    }
#endif

    /* now we compute B=X x A1 and C=Xs x A2 */
    for(int i=0;i<3;i++){
      for(int j=0;j<3;j++){
        element_init_G1(B[i][j],*pairing);
        element_init_G2(C[i][j],*pairing);
        for(int k=0;k<3;k++){
	        element_mul_zn(tmp1,A1[k][j],X[i][k]);
	        element_mul_zn(tmp2,A2[k][j],Xs[i][k]);
	        element_add(B[i][j],B[i][j],tmp1);
	        element_add(C[i][j],C[i][j],tmp2);
        }
      }
    }

    transpose(C,3);

    for(int i=0; i<3; ++i) {
        for(int j=0; j<3; ++j) {
	        element_clear(X[i][j]);
	        element_clear(Xs[i][j]);
      }
      free(X[i]);
      free(Xs[i]);
    }
    free(X);
    free(Xs);

  } /* for zz */


#ifdef VERBOSE
  element_t check, oneT;
  element_init_GT(check, *pairing);
  element_init_GT(oneT, *pairing);
  element_set1(oneT);
  for (int k = 0; k < l + 1; ++k) {
    printf("Checking: B[%d]=%p and C[%d]=%p\n",k,BB[k],k,CC[k]);
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
	element_prod_pairing(check, BB[k][i], CC[k][j], 3);
	if (i == j)
	  printf("[%d] %d == %d -> gT: %s\n", k, i, j, 0 == element_cmp(gT, check) ? "true" : "false");
	else
	  printf("[%d] %d != %d -> 1T: %s\n", k, i, j, 0 == element_cmp(oneT, check) ? "true" : "false");
      }
    }
  }
  element_clear(oneT);
  element_clear(check);
#endif

/*
  element_clear(tmp1); element_clear(tmp2);
  element_clear(delta); element_clear(t1);
  element_clear(t2); element_clear(t3);
  element_clear(t); element_clear(gT);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      element_clear(A1[i][j]);
      element_clear(A2[i][j]);
    }
    free(A1[i]);
    free(A2[i]);
  }
  free(A1);
  free(A2);
*/

  return setup;
}

Asetup_t *
Asetup(pairing_t *pairing, int l)
{

    element_t psi,g1,g2;

    /* pick random psi in the field */
    element_init_Zr(psi, *pairing); element_random(psi);

    /* any element of G1/G2 other than 0 is a generator*/
    element_init_G1(g1, *pairing); element_random(g1);
    element_init_G2(g2, *pairing); element_random(g2);

    Asetup_t *result=malloc(sizeof(Asetup));
#ifdef VERBOSE
    printf("Generating public/private key for l=%d...\n",l);
#endif
    setup_t sl=setup(pairing,psi,g1,g2,l);
#ifdef VERBOSE
    printf("Done\n");
#endif
#ifdef VERBOSE
    printf("Generating public/private key for l=1...\n");
#endif
    setup_t s1=setup(pairing,psi,g1,g2,1);
#ifdef VERBOSE
    printf("Done\n");
#endif
    result->publicl=sl->public; result->privatel=sl->private;
    result->public1=s1->public; result->private1=s1->private;

    return result;
}

/* attributes x[1],...,x[public->l] */
ct_t *
encrypt(pairing_t *pairing, mpk_t public, int *x, element_t *m)
{

    element_t v[3];
    int l=public->l;

#ifdef VERBOSE
    printf("Attributes for encrypting\n");
    for(int i=1;i<l+1;i++)
        printf("X[%d]=%d\n",i,x[i]);
#endif
#ifdef VERBOSE
    for(int t=0;t<l+1;t++){
        printf("Encrypting: B[%d]=%p\n",t,public->B[t]);
    }
#endif

    ct_t *ct=malloc(sizeof(ct_t));
    element_init_GT(ct->c,*pairing);

    element_t z;
    element_init_Zr(z,*pairing); element_random(z);
    element_pow_zn(ct->c,public->gT,z);
#ifdef VERBOSE
    element_printf("Encrypting: z %B\n",z);
    element_printf("Encrypting: gT^z %B\n",ct->c);
#endif
    element_mul(ct->c,ct->c,*m);
#ifdef VERBOSE
    element_printf("Encrypting: ct->c %B\n",ct->c);
#endif

    ct->ci=malloc(sizeof(element_t *)*(l+1));
    for(int t=0;t<l+1;t++){
        ct->ci[t]=malloc(sizeof(element_t)*3);
        for(int i=0;i<3;i++){
            element_init_G1(ct->ci[t][i],*pairing);
        }
    }

    element_t w0;
    element_init_Zr(w0,*pairing); element_random(w0);
    element_init_Zr(v[0],*pairing); element_init_Zr(v[1],*pairing);
    element_init_Zr(v[2],*pairing);
    element_set(v[0],w0); element_set(v[1],z); element_set0(v[2]);
    v_times_m(ct->ci[0],v,public->B[0]);

    element_t wt,xt;
    element_init_Zr(wt,*pairing); element_init_Zr(xt,*pairing);
    element_set(v[2],w0);
    for(int t=1;t<l+1;t++){
        element_random(wt);
        element_set(v[0],wt);
        element_set_si(xt,x[t]);
        element_mul(v[1],wt,xt);
        v_times_m(ct->ci[t],v,public->B[t]);
    }

    return ct;
}

/* common attributes in x from 1 to l x[0] is dummy
   personal attributes in x from l+2 to l+n+1
   personal attribute of m[i] in x[l+2+i]
   plaintexts m from 1 to n m[0] is dummy
*/
Act_t *
Aencrypt(pairing_t* pairing, mpk_t publicl, mpk_t public1, int *x, int n, element_t *m)
{


    int l=publicl->l;
    Act_t *act=malloc(sizeof(Act_t));
    act->l=l; act->n=n;

    element_t M;
    element_init_GT(M,*pairing); element_set1(M);
    ct_t *ctl, *ct1;
    ctl=encrypt(pairing,publicl,x,&M);
    act->ci=ctl->ci;

#ifdef VERBOSE
    element_printf("ctl->c=%B\n",ctl->c);
#endif

    act->c=malloc(sizeof(element_t)*(n+1));
    act->cs=malloc(sizeof(element_t *)*(n+1));
    act->cl=malloc(sizeof(element_t *)*(n+1));

    for(int j=1;j<n+1;j++){
        ct1=encrypt(pairing,public1,x+l+j,m+j);
        element_init_GT(act->c[j],*pairing);
        element_mul(act->c[j],ct1->c,ctl->c);
        act->cs[j]=malloc(sizeof(element_t)*3);
        act->cl[j]=malloc(sizeof(element_t)*3);
        for(int i=0;i<3;i++){
            element_init_G1(act->cs[j][i],*pairing);
            element_set(act->cs[j][i],ct1->ci[0][i]);
            element_init_G1(act->cl[j][i],*pairing);
            element_set(act->cl[j][i],ct1->ci[1][i]);
        }
    }

    return act;
}

/* attributes y[1],...,y[l],y[l+1]. y[0] is dummy */
/* y[t]=-1 --> y_t=*    y[l+1] is special */

Akey_t  *
Akeygen(pairing_t *pairing, msk_t privatel, msk_t private1,char *condition, int *y)
{

    int l=privatel->l;
    Akey_t *akey=malloc(sizeof(Akey_t));
    akey->l=l;
    akey->n=y[l+1];
    akey->cond = condition;

    dkey_t keyl=keygen(pairing,privatel,y);
    akey->kl=keyl->k;
    private1->l=1;
    dkey_t key1=keygen(pairing,private1,y+l);
    akey->k1=key1->k;
    return akey;
}

void
storeAkey(pairing_t *pairing, Akey_t *token, FILE *fout)
{
    fprintf(fout,"%d\n",token->l);
    fprintf(fout,"%d\n",token->n);
    fprintf(fout,"%s\n",token->cond);
    for (int i=0;i<token->l+1;i++)
        if (token->kl[i]){
            for (int j=0;j<3;j++)
                element_fprintf(fout,"%B\n",token->kl[i][j]);
        } else
            fprintf(fout,"\n");
    for (int i=0;i<2;i++)
        for (int j=0;j<3;j++)
            element_fprintf(fout,"%B\n",token->k1[i][j]);

}

Akey_t *
readAkey(pairing_t *pairing, FILE *fin)
{

    char *elementS=NULL;
    int len;

    Akey_t *token=malloc(sizeof(Akey_t));
    getline(&elementS,&len,fin); token->l=atoi(elementS);
#ifdef VERBOSE
    printf("readAkey*l=%d*%s*\n",token->l,elementS);
#endif
    getline(&elementS,&len,fin); token->n=atoi(elementS);
#ifdef VERBOSE
    printf("readAkey*n=%d*%s*\n",token->n,elementS);
#endif
    getline(&elementS,&len,fin); token->cond = malloc(sizeof(char)*strlen(elementS)+1); sprintf(token->cond,"%s",elementS);
    token->kl=malloc((token->l+1)*sizeof(element_t *));
    for (int i=0;i<token->l+1;i++){
        getline(&elementS,&len,fin);
        if (elementS[0]=='\n'){
            token->kl[i]=NULL;
            continue;
        }
        elementS[strlen(elementS)-1]='\0';
        token->kl[i]=malloc(3*sizeof(element_t));
        element_init_G2(token->kl[i][0],*pairing);
        element_set_str(token->kl[i][0],elementS,10);
        for (int j=1;j<3;j++){
            getline(&elementS,&len,fin);
            elementS[strlen(elementS)-1]='\0';
            element_init_G2(token->kl[i][j],*pairing);
            element_set_str(token->kl[i][j],elementS,10);
        }
    }
    token->k1=malloc(2*sizeof(element_t *));
    for (int i=0;i<2;i++){
        token->k1[i]=malloc(3*sizeof(element_t));
        for (int j=0;j<3;j++){
            getline(&elementS,&len,fin);
            elementS[strlen(elementS)-1]='\0';
            element_init_G2(token->k1[i][j],*pairing);
            element_set_str(token->k1[i][j],elementS,10);
        }
    }

    return token;
}

/* attributes y[1],...,y[l]. y[0] is dummy */
/* y[t]=-1 --> y_t=* */
dkey_t
keygen(pairing_t *pairing, msk_t private, int *y)
{

    int l=private->l;

#ifdef VERBOSE
    printf("Attributes for keygen\n");
    for(int i=1;i<l+1;i++)
        printf("Y[%d]=%d\n",i,y[i]);
#endif
#ifdef VERBOSE
    for(int t=0;t<l+1;t++){
        printf("Keygen: C[%d]=%p\n",t,private->C[t]);
    }
#endif
    dkey_t k=malloc(sizeof(*k));
    k->l=l;
    k->k=malloc(sizeof(element_t *)*(l+1));

    element_t yt, dt, st, s0, v[3];
    element_init_Zr(v[0],*pairing); element_init_Zr(v[1],*pairing);
    element_init_Zr(v[2],*pairing); element_init_Zr(dt,*pairing);
    element_init_Zr(yt,*pairing);   element_init_Zr(s0,*pairing);
    element_init_Zr(st,*pairing);   element_set0(s0);

    for(int t=1;t<l+1;++t) {
        if(-1==y[t]){k->k[t]=NULL; continue;}

        k->k[t]=malloc(sizeof(element_t)*3);
        element_init_G2(k->k[t][0],*pairing);
        element_init_G2(k->k[t][1],*pairing);
        element_init_G2(k->k[t][2],*pairing);
        element_random(dt); element_random(st);
        element_add(s0,s0,st); // sum st
        element_set_si(yt,y[t]);
        element_mul(v[0],dt,yt);
        element_neg(v[1],dt);
        element_set(v[2],st); // (d_t \times y_t, -d_t, s_t)
        v_times_m(k->k[t],v,private->C[t]);
    }

    element_neg(s0, s0); // s0 = -(\sum_{t \in S} s_t)
#ifdef VERBOSE
    element_printf("Keygen: s0=%B\n",s0);
#endif
    element_set(v[0], s0);
    element_set1(v[1]);
    element_random(v[2]); // (s_0, 1, eta)
    k->k[0]=malloc(sizeof(element_t)*3);
    element_init_G2(k->k[0][0],*pairing);
    element_init_G2(k->k[0][1],*pairing);
    element_init_G2(k->k[0][2],*pairing);
    v_times_m(k->k[0],v,private->C[0]);

    return k;
}

element_t *
decrypt(pairing_t *pairing, ct_t *ct, dkey_t key)
{

    int l=key->l;
    element_t *m = malloc(sizeof(element_t));
    element_init_GT(*m,*pairing);
#ifdef VERBOSE
    element_t TTT;
    element_init_GT(TTT,*pairing);
    element_prod_pairing(TTT,ct->ci[0],key->k[0],3);
    element_printf("TTT: %B\n",TTT);
#endif

    element_t bigProd, tmp1, tmp2;
    element_init_GT(bigProd,*pairing); element_set1(bigProd);
    element_init_GT(tmp1,*pairing); element_init_GT(tmp2, *pairing);

    for (int t=0;t<l+1;t++){
        if (NULL==key->k[t]) continue;
        element_prod_pairing(tmp1,ct->ci[t],key->k[t],3);
#ifdef VERBOSE
    element_printf("e(ct,kt): %B\n",tmp1);
#endif
        element_mul(bigProd,bigProd,tmp1);
    }

#ifdef VERBOSE
    element_printf("bigProd: %B\n",bigProd);
    element_printf("ct-c: %B\n",ct->c);
#endif
    element_div(*m,ct->c,bigProd);
  return m;
}

element_t *
ASingdecrypt(pairing_t *pairing, Act_t *ct, Akey_t *key, int j){

    int n=ct->n;
    int l=key->l;
    element_t *m = malloc(sizeof(element_t));

    element_t bigProd, tmp1, tmp2;
    element_init_GT(bigProd,*pairing); element_set1(bigProd);
    element_init_GT(tmp1,*pairing); element_init_GT(tmp2, *pairing);

    for (int t=0;t<l+1;t++){
        if (NULL==key->kl[t]) continue;
	if (NULL==ct->ci[t] ) continue;
        element_prod_pairing(tmp1,ct->ci[t],key->kl[t],3);
        element_mul(bigProd,bigProd,tmp1);
    }

#ifdef VERBOSE
    element_printf("bigProd: %B\n",bigProd);
#endif
        element_prod_pairing(tmp1,ct->cs[j],key->k1[0],3);
        element_prod_pairing(tmp2,ct->cl[j],key->k1[1],3);
        element_mul(tmp1,tmp1,tmp2);
        element_init_GT(*m,*pairing);
        element_div(*m,ct->c[j],bigProd);
        element_div(*m,*m,tmp1);
  return m;
}

element_t *
Adecrypt(pairing_t *pairing, Act_t *ct, Akey_t *key){

    int n=ct->n;
    int l=key->l;
    element_t *m = malloc(sizeof(element_t)*(n+1));

    element_t bigProd, tmp1, tmp2;
    element_init_GT(bigProd,*pairing); element_set1(bigProd);
    element_init_GT(tmp1,*pairing); element_init_GT(tmp2, *pairing);

    for (int t=0;t<l+1;t++){
        if (NULL==key->kl[t]) continue;
        element_prod_pairing(tmp1,ct->ci[t],key->kl[t],3);
        element_mul(bigProd,bigProd,tmp1);
    }

#ifdef VERBOSE
    element_printf("bigProd: %B\n",bigProd);
#endif
    for (int j=1;j<n+1;j++){
        element_prod_pairing(tmp1,ct->cs[j],key->k1[0],3);
        element_prod_pairing(tmp2,ct->cl[j],key->k1[1],3);
        element_mul(tmp1,tmp1,tmp2);
        element_init_GT(m[j],*pairing);
        element_div(m[j],ct->c[j],bigProd);
        element_div(m[j],m[j],tmp1);
    }
  return m;
}



void
storeMpk(pairing_t *pairing, mpk_t key, FILE *fout){

    int l=key->l;
    fprintf(fout,"%d\n",l);
    element_t ***B=key->B;
    for(int zz=0;zz<l+1;zz++){
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                element_fprintf(fout,"%B\n",B[zz][i][j]);
            }
        }
    }
    element_fprintf(fout,"%B\n",key->gT);

}

void
storeMsk(pairing_t *pairing, msk_t key, FILE *fout){

    int l=key->l;
    fprintf(fout,"%d\n",l);
    element_t ***C=key->C;
    for(int zz=0;zz<l+1;zz++)
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++)
                element_fprintf(fout,"%B\n",C[zz][i][j]);

}

void
storeAMpk(pairing_t *pairing, Asetup_t *setUp, FILE *fout)
{
    storeMpk(pairing,setUp->publicl,fout);
    storeMpk(pairing,setUp->public1,fout);
}

void
storeSetup(pairing_t *pairing, Asetup_t *key, FILE *fout)
{

    storeMpk(pairing,key->publicl,fout);
    storeMpk(pairing,key->public1,fout);
    storeMsk(pairing,key->privatel,fout);
    storeMsk(pairing,key->private1,fout);

}

//typedef struct {
  //unsigned int l;
  //unsigned int n;
  //element_t *c; /* n elements from GT */
  //element_t **ci; /* l+1 vectors of 3 elements of G1 */
  //element_t **cs; /* n vectors of 3 elements of G1 */
  //element_t **cl; /* n vectors of 3 elements of G1 */
//} Act_t;

Act_t *
readAct(pairing_t *pairing,  FILE *fin)
{

    int i,j,n,l;
    size_t len=0;
    char *elementS=NULL;

    Act_t *CT=malloc(sizeof(Act_t));

    getline(&elementS,&len,fin); l=atoi(elementS);
    free(elementS);
    elementS=NULL;
//    l=6;
    getline(&elementS,&len,fin); n=atoi(elementS);
//    n=6;
    CT->l=l; CT->n=n;

    CT->c=malloc((n+1)*sizeof(element_t));
    for(i=1;i<CT->n+1;i++){
        getline(&elementS,&len,fin);
        elementS[strlen(elementS)-1]='\0';
        element_init_GT(CT->c[i],*pairing);
        element_set_str(CT->c[i],elementS,10);
    }

    CT->cs=malloc((n+1)*sizeof(element_t *));
    for(i=1;i<CT->n+1;i++){
        CT->cs[i]=malloc(3*sizeof(element_t));
        for(j=0;j<3;j++){
            getline(&elementS,&len,fin);
            elementS[strlen(elementS)-1]='\0';
            element_init_G1(CT->cs[i][j],*pairing);
            element_set_str(CT->cs[i][j],elementS,10);
        }
    }

    CT->cl=malloc((n+1)*sizeof(element_t *));
    for(i=1;i<CT->n+1;i++){
        CT->cl[i]=malloc(3*sizeof(element_t));
        for(j=0;j<3;j++){
            getline(&elementS,&len,fin);
            elementS[strlen(elementS)-1]='\0';
            element_init_G1(CT->cl[i][j],*pairing);
            element_set_str(CT->cl[i][j],elementS,10);
        }
    }

    CT->ci=malloc((l+1)*sizeof(element_t *));
    for(i=0;i<CT->l+1;i++){
        CT->ci[i]=malloc(3*sizeof(element_t));
        for(j=0;j<3;j++){
            getline(&elementS,&len,fin);
            elementS[strlen(elementS)-1]='\0';
            element_init_G1(CT->ci[i][j],*pairing);
            element_set_str(CT->ci[i][j],elementS,10);
	    free(elementS);
	    elementS=NULL;
            len=0;
        }
    }

    return CT;
}

void
storeAct(pairing_t *pairing, Act_t *CT, FILE *fout)
{

    int i,j;

    fprintf(fout,"%d\n",CT->l);
    fprintf(fout,"%d\n",CT->n);
    for(i=1;i<CT->n+1;i++){
        element_fprintf(fout,"%B\n",CT->c[i]);
    }

    for(i=1;i<CT->n+1;i++){
        for(j=0;j<3;j++){
            element_fprintf(fout,"%B\n",CT->cs[i][j]);
        }
    }

    for(i=1;i<CT->n+1;i++){
        for(j=0;j<3;j++){
            element_fprintf(fout,"%B\n",CT->cl[i][j]);
        }
    }

    for(i=0;i<CT->l+1;i++){
        for(j=0;j<3;j++){
            element_fprintf(fout,"%B\n",CT->ci[i][j]);
        }
    }

}

mpk_t
readMpk(pairing_t *pairing, FILE *fin)
{

    char *buf=NULL;
    int l, len;
    mpk_t key;

    key=malloc(sizeof(*key));
    len=0;
    getline(&buf,&len,fin);
    l=atoi(buf); key->l=l;
    key->B=malloc((l+1)*sizeof(element_t **));
    for (int zz=0;zz<l+1;zz++){
        key->B[zz]=malloc(3*sizeof(element_t*));
        key->B[zz][0]=malloc(3*sizeof(element_t));
        key->B[zz][1]=malloc(3*sizeof(element_t));
        key->B[zz][2]=malloc(3*sizeof(element_t));
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                getline(&buf,&len,fin);
                buf[strlen(buf)-1]='\0';
#ifdef VERBOSEA
                printf("PRead: %d %d %d\n%s\n",zz,i,j,buf);
#endif
                element_init_G1(key->B[zz][i][j],*pairing);
                element_set_str(key->B[zz][i][j],buf,10);
            }
        }
    }
     element_init_GT(key->gT,*pairing);
     getline(&buf,&len,fin);
     buf[strlen(buf)-1]='\0';
     element_set_str(key->gT,buf,10);
     return key;
}

msk_t
readMsk(pairing_t *pairing, FILE *fin){

    char *buf=NULL;
    int l,len;

    msk_t key=malloc(sizeof(msk_t*));
    getline(&buf,&len,fin); l=atoi(buf); key->l=l;
#ifdef VERBOSE
    printf("readMsk\tl=%d\n",l);
#endif
    key->C=malloc((l+1)*sizeof(element_t**));
    for (int zz=0;zz<l+1;zz++){
        key->C[zz]=malloc(3*sizeof(element_t*));
        key->C[zz][0]=malloc(3*sizeof(element_t));
        key->C[zz][1]=malloc(3*sizeof(element_t));
        key->C[zz][2]=malloc(3*sizeof(element_t));
        for(int i=0;i<3;i++){
            for(int j=0;j<3;j++){
                getline(&buf,&len,fin);
                buf[strlen(buf)-1]='\0';
#ifdef VERBOSE
                printf("readMsk\tzz=%d i=%d z=%d\nbuf=%s\n",zz,i,j,buf);
#endif
                element_init_G2(key->C[zz][i][j],*pairing);
                element_set_str(key->C[zz][i][j],buf,10);
            }
        }
    }

    return key;
}



Asetup_t *
readAMpk(pairing_t *pairing, FILE *fin)
{
    Asetup_t *setUp;
    setUp=malloc(sizeof(Asetup_t));
    setUp->publicl=readMpk(pairing,fin);
    setUp->public1=readMpk(pairing,fin);
    setUp->l=setUp->publicl->l;
    setUp->privatel=NULL;
    setUp->private1=NULL;
    return setUp;
}

Asetup_t *
readSetup(pairing_t *pairing, FILE *fin)
{
    Asetup_t *setUp;
    setUp=malloc(sizeof(Asetup_t));
    setUp->publicl=readMpk(pairing,fin);
    setUp->public1=readMpk(pairing,fin);
    setUp->privatel=readMsk(pairing,fin);
    setUp->private1=readMsk(pairing,fin);
    setUp->l=setUp->publicl->l;
    return setUp;

}

