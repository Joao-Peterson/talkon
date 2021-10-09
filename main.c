#include <stdio.h>
#include <stdlib.h>
#include "inc/doc.h"
#include "inc/doc_json.h"

int main(int argc, char **argv){

    doc *json = doc_new("json", dt_obj, 
        "sample", dt_const_string, "Hello this a json!", 19ULL,
        ";"
    );

    if(doc_error_code){
        printf("%s\n", doc_get_error_msg());
    }

    doc_json_save(json, "out.json");
    
    return 0;
}