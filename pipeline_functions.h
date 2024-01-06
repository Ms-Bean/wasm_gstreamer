/*A set of demo pipeline functions*/
/*The pipeline implementation supports the execution of any functions with this structure,
  as such it could be used for a wide variety of pipeline-based systems*/

/*Since calls to malloc are made, `pipeline_free_outputs` should be called to deallocate the
  memory segments allocated here.*/

void add(void **inputs, void **params, void ***outputs)
{
    /*Get inputs*/
    int a = *((int *)inputs[0]);
    int b = *((int *)inputs[1]);

    /*Send outputs*/
    *(outputs[0]) = malloc(sizeof(int));
    if(*(outputs[0]) == NULL)
    {
        fprintf(stderr, "add: malloc error\n");
        exit(4);
    }
    *((int *)(*(outputs[0]))) = a + b;
}

void number_source(void **inputs, void **params, void ***outputs)
{
    /*Get internal parameters*/
    int num = *((int *)params[0]);

    /*Send outputs*/
    *(outputs[0]) = malloc(sizeof(int));
    if(*(outputs[0]) == NULL)
    {
        fprintf(stderr, "main: malloc error\n");
        exit(4);
    }
    *((int *)(*(outputs[0]))) = num;
}

void nop(void **inputs, void **params, void ***outputs)
{
    return;
}