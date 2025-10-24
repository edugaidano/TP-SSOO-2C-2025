#ifndef SET_INSTRUCCIONES_H
#define SET_INSTRUCCIONES_H

typedef enum set_instrucciones {
    CREATE,
    TRUNCATE,
    WRITE,
    FLUSH,
    COMMIT,
    READ,
    TAG,
    DELETE,
    END
} set_instrucciones;

#endif