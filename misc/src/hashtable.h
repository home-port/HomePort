// hashtabel.h

struct nlist { /* table entry: */
     struct nlist *next; /* next entry in chain */
     char *name; /* defined name */
     char *defn; /* replacement text */
};

struct nlist *lookup(char *s, struct nlist **hashtable);

struct nlist *install(char *name, char *defn, struct nlist **hashtable);
