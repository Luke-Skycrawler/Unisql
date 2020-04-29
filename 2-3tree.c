#include <stdio.h>
#include <stdlib.h>
// insert with splitting
typedef struct node{
    int k[3],n,ischild;
    struct node *p[3];
} tree;
// k[0] to store the smallest element in the subtree
static tree *root=NULL,*stack[20];
static int cursor=-1,stack_int[20];
// int tmp[4];
// for extension
void push(tree *x,int j){
    stack[++cursor]=x;
    stack_int[cursor]=j;
}
tree *pop(int *j){
    if(cursor==-1)return NULL;
    *j=stack_int[cursor];
    return stack[cursor--];
}
void link(tree *r,tree *child,int j){
    // root isn't full
    int i;
    for(i=r->n-1;i>j;i--){
        r->p[i+1]=r->p[i];
        r->k[i+1]=r->k[i];
    }
    r->p[j+1]=child;
    r->k[j+1]=child->k[0];
    r->n++;
}
int insert(int x){
    tree *p,*q,*sibling,*parent;
    int i,*a,j;
    p=root;
    if(p==NULL){
        root=malloc(sizeof(tree));
        root->ischild=1;
        root->n=1;
        root->k[0]=x;
        return 0;
    }
    while(!(p->ischild)){
        for(i=1;i<p->n;i++)
            if(x<p->k[i])break;
            else if(x==p->k[i]){
                cursor=-1;
                return -1;
            }
        push(p,--i);
        p=p->p[i];
    }
    a=p->k;
    // and should contain index of the child branch
    if(p->n<3){
        for(i=p->n-1;i>=0;i--)
            if(a[i]<x)break;
                else if(a[i]==x){
                    cursor=-1;
                    return -1;
                }
                else a[i+1]=a[i];
        p->n++;
        parent=pop(&j);
        if(parent&&(i<0))parent->k[j]=x;
        a[i+1]=x;
    }
    else{
        // split, leaf
        for(i=0;i<p->n;i++)if(x==p->k[i]){
            cursor=-1;
            return -1;
        }
        q=malloc(sizeof(tree));
        p->n=q->n=2;
        q->ischild=1;
        if(a[1]>x){
            q->k[0]=a[1];q->k[1]=a[2];
            if(x>a[0])
                a[1]=x;
            else {
                a[1]=a[0];
                a[0]=x;
            }
        }
        else {
            q->k[0]=x>a[2]?a[2]:x;
            q->k[1]=x+a[2]-q->k[0];
        }
        // those scopes shall be modified if order is extended
        parent=pop(&j);
        // obscure part
        if(!parent){
            p=root;
            sibling=q;
        }
        // TODO:pointer and key values trans
        // TODO:pop all elements before return
        // indexed before j steady
        while(parent&&parent->n>=3){
            // dangerous, split iteratively
            p=parent;
            sibling=malloc(sizeof(tree));
            sibling->n=p->n=2;
            sibling->ischild=0;
            if(j>=2){
                sibling->p[0]=p->p[2];
                sibling->k[0]=p->k[2];
                sibling->p[1]=q;
                sibling->k[1]=q->k[0];
            }
            else {
                if(j==0){
                    sibling->p[0]=p->p[1];
                    sibling->k[0]=p->k[1];
                    p->p[1]=q;
                    p->k[1]=q->k[0];
                }
                else {
                    sibling->p[0]=q;
                    sibling->k[0]=q->k[0];
                }
                sibling->p[1]=p->p[2];
                sibling->k[1]=p->k[2];
            }
            parent=pop(&j);
            if(parent==NULL)break;
            q=sibling;
        }
        if(parent==NULL){
            q=malloc(sizeof(tree));
            q->ischild=0;
            q->n=2;
            q->p[0]=p;
            q->p[1]=sibling;
            q->k[0]=p->k[0];
            q->k[1]=sibling->k[0];
            root=q;
            cursor=-1;
            return 0;
        }
        link(parent,q,j);
    }
    cursor=-1;
    return 0;
}
FILE *f,*o;
void printnode(tree *p){
    int i,n;
    fprintf(o,"[");
    n=p->n-1;
    for(i=p->ischild?0:1;i<n;i++)fprintf(o,"%d,",p->k[i]);
    fprintf(o,"%d]",p->k[n]);
}
#define cap 10100
static tree *a[cap];
static int start=0,end=0;
tree *deque(void){
    if(end==start)return NULL;
    if(start<cap)return a[start++];
    start=0;
    return a[cap];
}
void enque(tree *t){
    if(end==cap){
        a[cap]=t;
        end=0;
    }
    else a[end++]=t;
}

void visual(tree *r){
    tree *p;
    int i;
    enque(r);
    enque(NULL);
    do{
        p=deque();
        if(p==NULL){
            // printf("\n");
            enque(NULL);
            p=deque();
            if(p==NULL)break;
        }
        if(!p->ischild)
            for(i=0;i<p->n;i++)enque(p->p[i]);
        else printnode(p);
    }
    while(1);
}
int main(void){
    int n,i,x,e;
    f=fopen("input.txt","r");
    o=fopen("output.txt","w");
    fscanf(f,"%d",&n);
    for(i=0;i<n;i++){
        fscanf(f,"%d",&x);
        e=insert(x);
        if(e==-1)
            fprintf(o,"Key %d is duplicated\n",x);
    }
    visual(root);
    fclose(f);
    fclose(o);
    return 0;
}