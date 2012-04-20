#ifndef BIGINTMAT_H
#define BIGINTMAT_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/*
* ABSTRACT: class bigintmat: matrizes of big integers
*/
#include <string.h>
#include <omalloc/omalloc.h>
#include <coeffs/coeffs.h>
#include <coeffs/longrat.h>
#include <misc/intvec.h>


class bigintmat
{
private:
  number *v;
  int row;
  int col;
public:

  bigintmat()
  {
    row = 1;
    col = 0;
    v = NULL;
  }
  bigintmat(int r, int c);

  bigintmat(const bigintmat *m)
  {
    row = m->rows();
    col = m->cols();
    if (row*col>0)
    {
      v = (number *)omAlloc(sizeof(number)*row*col);
      for (int i=row*col-1; i>=0; i--)
      {
        v[i] = nlCopy((*m)[i],NULL);
      }
    }
  }


  inline number& operator[](int i)
    {
#ifndef NDEBUG
      if((i<0)||(i>=row*col))
      {
        Werror("wrong bigintmat index:%d\n",i);
      }
#endif
      return v[i];  // Hier sollte imho kein nlCopy rein...
    }
  inline const number& operator[](int i) const
    {
#ifndef NDEBUG
      if((i<0)||(i>=row*col))
      {
        Werror("wrong bigintmat index:%d\n",i);
      }
#endif
      return v[i];
    }

#define BIMATELEM(M,I,J) (M)[(I-1)*(M).cols()+J-1]
  void operator*=(int intop);
  void operator*=(number bintop);
  inline int  cols() const { return col; }
  inline int  rows() const { return row; }
  inline ~bigintmat()
    {
      if (v!=NULL)
      {
        for (int i=0; i<row*col; i++) { nlDelete(&(v[i]), NULL); }
        omFreeSize((ADDRESS)v,sizeof(number)*row*col);
        v=NULL;
      }
    }
  number get(int i, int j);
  number get(int i);
  void set(int i, int j, number n);
  void set(int i, number n);
  char * String();
  void pprint(int maxwid);
  int compare(const bigintmat* op) const;
  int getwid(int maxwid);
};
bool operator==(bigintmat & lhr, bigintmat & rhr);
bool operator!=(bigintmat & lhr, bigintmat & rhr);
bigintmat * bimAdd(bigintmat * a, bigintmat * b);
bigintmat * bimSub(bigintmat * a, bigintmat * b);
bigintmat * bimMult(bigintmat * a, bigintmat * b);
intvec * bim2iv(bigintmat * b);
bigintmat * bimCopy(const bigintmat * b);
static void bimRowContent(bigintmat *bimat, int rowpos, int colpos);
static void bimReduce(bigintmat *bimat, int rpiv, int colpos,
                     int ready, int all);

bigintmat * iv2bim(intvec * b);
#endif