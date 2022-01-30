/* PMSIS includes */
#include "pmsis.h"

#define MATRIX_WIDTH    (64)
#define MATRIX_HEIGHT   (64)
#define MATRIX_SIZE     (MATRIX_WIDTH * MATRIX_HEIGHT)

struct cl_args_s
{
    uint32_t size;
    unsigned short *mat1_l2;
    unsigned short *mat2_l2;
    unsigned short *mat_add_l2;
    unsigned short *mat_mult_l2;
    unsigned short *mat1_l1;
    unsigned short *mat2_l1;
    unsigned short *mat_add_l1;
    unsigned short *mat_mult_l1;
};

PI_L2 static struct cl_args_s cl_arg;

/* Task executed by cluster cores. */
void cluster_dma(void *arg)
{
    struct cl_args_s *dma_args = (struct cl_args_s *) arg;
    
    uint32_t buffer_size        = dma_args->size;
    unsigned short *mat1_l2     = dma_args->mat1_l2;
    unsigned short *mat2_l2     = dma_args->mat2_l2;
    unsigned short *mat_add_l2  = dma_args->mat_add_l2;
    unsigned short *mat_mult_l2 = dma_args->mat_mult_l2;
    unsigned short *mat1_l1     = dma_args->mat1_l1;
    unsigned short *mat2_l1     = dma_args->mat2_l1;
    unsigned short *mat_add_l1  = dma_args->mat_add_l1;
    unsigned short *mat_mult_l1 = dma_args->mat_mult_l1;

    uint32_t coreid = pi_core_id(), start = 0, end = 0;

    /* Core 0 of cluster initiates DMA transfer from L2 to L1. */
    if (!coreid)
    {
        printf("Core %d requesting Matrix1 DMA transfer from L2 to L1.\n", coreid);
        pi_cl_dma_copy_t copy_mat1;
        copy_mat1.dir = PI_CL_DMA_DIR_EXT2LOC;
        copy_mat1.merge = 0;
        copy_mat1.size = buffer_size;
        copy_mat1.id = 0;
        copy_mat1.ext = (uint32_t) mat1_l2;
        copy_mat1.loc = (uint32_t) mat1_l1;

        pi_cl_dma_memcpy(&copy_mat1);
        pi_cl_dma_wait(&copy_mat1);
        printf("Core %d : Matrix1 Transfer done.\n", coreid);

        printf("Core %d requesting Matrix2 DMA transfer from L2 to L1.\n", coreid);
        pi_cl_dma_copy_t copy_mat2;
        copy_mat2.dir = PI_CL_DMA_DIR_EXT2LOC;
        copy_mat2.merge = 0;
        copy_mat2.size = buffer_size;
        copy_mat2.id = 0;
        copy_mat2.ext = (uint32_t) mat2_l2;
        copy_mat2.loc = (uint32_t) mat2_l1;

        pi_cl_dma_memcpy(&copy_mat2);
        pi_cl_dma_wait(&copy_mat2);
        printf("Core %d : Matrix2 Transfer done.\n", coreid);
    }

    start = (coreid * (buffer_size / pi_cl_cluster_nb_pe_cores()));
    end = (start - 1 + (buffer_size / pi_cl_cluster_nb_pe_cores()));

    /* Barrier synchronisation before starting to compute. */
    pi_cl_team_barrier(0);
    /* Each core computes on specific portion of buffer. */
    for (uint32_t i=start; i<=end; i++)
    {
        mat_add_l1[i] = mat1_l1[i] + mat1_l2[i];
        mat_mult_l1[i] = mat1_l1[i] * mat1_l2[i];
    }
    /* Barrier synchronisation to wait for all cores. */
    pi_cl_team_barrier(0);

    /* Core 0 of cluster initiates DMA transfer from L1 to L2. */
    if (!coreid)
    {
        printf("Core %d requesting Matrix addd DMA transfer from L1 to L2.\n", coreid);
        pi_cl_dma_copy_t copy_mat_add;
        copy_mat_add.dir = PI_CL_DMA_DIR_LOC2EXT;
        copy_mat_add.merge = 0;
        copy_mat_add.size = buffer_size;
        copy_mat_add.id = 0;
        copy_mat_add.ext = (uint32_t) mat_add_l2;
        copy_mat_add.loc = (uint32_t) mat_add_l1;

        pi_cl_dma_memcpy(&copy_mat_add);
        pi_cl_dma_wait(&copy_mat_add);
        printf("Core %d : Matrix add Transfer done.\n", coreid);

        printf("Core %d requesting Matrix mult DMA transfer from L1 to L2.\n", coreid);
        pi_cl_dma_copy_t copy_mat_mult;
        copy_mat_mult.dir = PI_CL_DMA_DIR_LOC2EXT;
        copy_mat_mult.merge = 0;
        copy_mat_mult.size = buffer_size;
        copy_mat_mult.id = 0;
        copy_mat_mult.ext = (uint32_t) mat_mult_l2;
        copy_mat_mult.loc = (uint32_t) mat_mult_l1;

        pi_cl_dma_memcpy(&copy_mat_mult);
        pi_cl_dma_wait(&copy_mat_mult);
        printf("Core %d : Matrix mult Transfer done.\n", coreid);
    }
}

/* Cluster main entry, executed by core 0. */
void master_entry(void *arg)
{
    printf("Cluster master core entry\n");
    /* Task dispatch to cluster cores. */
    pi_cl_team_fork(pi_cl_cluster_nb_pe_cores(), cluster_dma, arg);
    printf("Cluster master core exit\n");
}

void MatCompute(void)
{
    printf("Entering main controller\n");
    uint32_t errors = 0;
    struct pi_device cluster_dev;
    struct pi_cluster_conf conf;

    uint32_t nb_cl_pe_cores = pi_cl_cluster_nb_pe_cores();
    uint32_t buffer_size = MATRIX_SIZE * sizeof(unsigned short);
    unsigned short *mat1_l2 = pi_l2_malloc(buffer_size);
    if (mat1_l2 == NULL)
    {
        printf("mat1_l2 buffer alloc failed !\n");
        pmsis_exit(-1);
    }

    unsigned short *mat2_l2 = pi_l2_malloc(buffer_size);
    if (mat2_l2 == NULL)
    {
        printf("mat1_l2 buffer alloc failed !\n");
        pmsis_exit(-1);
    }

    unsigned short *mat_add_l2  = pi_l2_malloc(buffer_size);
    if (mat_add_l2 == NULL)
    {
        printf("mat_add_l2 buffer alloc failed !\n");
        pmsis_exit(-1);
    }
    unsigned short *mat_mult_l2 = pi_l2_malloc(buffer_size);
    if (mat_mult_l2 == NULL)
    {
        printf("mat_mult_l2 buffer alloc failed !\n");
        pmsis_exit(-1);
    }


    /* L2 Array Init. */
    for (uint32_t i=0; i<buffer_size; i++)
    {
        mat1_l2[i] = i;
        mat2_l2[i] = 0;
        mat_add_l2[i] = 0;
        mat_mult_l2[i] = 0;

    }

    /* Init cluster configuration structure. */
    pi_cluster_conf_init(&conf);
    conf.id = 0;                /* Set cluster ID. */
    /* Configure & open cluster. */
    pi_open_from_conf(&cluster_dev, &conf);
    if (pi_cluster_open(&cluster_dev))
    {
        printf("Cluster open failed !\n");
        pmsis_exit(-3);
    }

    unsigned short *mat1_l1 = pi_cl_l1_malloc(&cluster_dev, buffer_size);
    if (mat1_l1 == NULL)
    {
        printf("mat1_l1 alloc failed !\n");
        pi_cluster_close(&cluster_dev);
        pmsis_exit(-4);
    }
    
    unsigned short *mat2_l1 = pi_cl_l1_malloc(&cluster_dev, buffer_size);
    if (mat2_l1 == NULL)
    {
        printf("mat2_l1 alloc failed !\n");
        pi_cluster_close(&cluster_dev);
        pmsis_exit(-4);
    }
    
    unsigned short *mat_add_l1 = pi_cl_l1_malloc(&cluster_dev, buffer_size);
    if (mat_add_l1 == NULL)
    {
        printf("mat_add_l1 alloc failed !\n");
        pi_cluster_close(&cluster_dev);
        pmsis_exit(-4);
    }

    unsigned short *mat_mult_l1 = pi_cl_l1_malloc(&cluster_dev, buffer_size);
    if (mat_mult_l1 == NULL)
    {
        printf("mat_mult_l1 alloc failed !\n");
        pi_cluster_close(&cluster_dev);
        pmsis_exit(-4);
    }


    /* Init arg struct. */

    cl_arg.size         = buffer_size;
    cl_arg.mat1_l2      = mat1_l2;
    cl_arg.mat2_l2      = mat2_l2;
    cl_arg.mat_add_l2   = mat_add_l2;
    cl_arg.mat_mult_l2  = mat_mult_l2;
    cl_arg.mat1_l1      = mat1_l1;
    cl_arg.mat2_l1      = mat2_l1;
    cl_arg.mat_add_l1   = mat_add_l1;
    cl_arg.mat_mult_l1  = mat_mult_l1;

    /* Prepare cluster task and send it to cluster. */
    struct pi_cluster_task *task = pi_l2_malloc(sizeof(struct pi_cluster_task));
    if (task == NULL)
    {
        printf("Cluster task alloc failed !\n");
        pi_cluster_close(&cluster_dev);
        pmsis_exit(-5);
    }
    memset(task, 0, sizeof(struct pi_cluster_task));
    task->entry = master_entry;
    task->arg = &cl_arg;

    printf("Sending task.\n");
    #if defined(ASYNC)
    pi_task_t wait_task;
    pi_task_block(&wait_task);
    pi_cluster_send_task_to_cl_async(&cluster_dev, task, &wait_task);
    printf("Wait end of cluster task\n");
    pi_task_wait_on(&wait_task);
    printf("End of cluster task\n");
    #else
    pi_cluster_send_task_to_cl(&cluster_dev, task);
    #endif  /* ASYNC */

    pi_l2_free(task, sizeof(struct pi_cluster_task));
    pi_cl_l1_free(&cluster_dev, mat1_l1, buffer_size);
    pi_cl_l1_free(&cluster_dev, mat2_l1, buffer_size);
    pi_cl_l1_free(&cluster_dev, mat_add_l1, buffer_size);
    pi_cl_l1_free(&cluster_dev, mat_mult_l1, buffer_size);

    printf("Close cluster after end of computation.\n");
    pi_cluster_close(&cluster_dev);

    
    // Verification.
    for (uint32_t i=0; i<buffer_size; i++)
    {
        if (mat_add_l2[i] != (unsigned short) (mat1_l2[i] + mat2_l2[i]))
        {
            printf("Error in add matrix\n");
            break;
        }
        if (mat_mult_l2[i] != (unsigned short) (mat1_l2[i] * mat2_l2[i]))
        {
            printf("Error in add matrix\n");
            break;
        }
    }
    

    pi_l2_free(mat1_l2, buffer_size);
    pi_l2_free(mat2_l2, buffer_size);
    pi_l2_free(mat_add_l2, buffer_size);
    pi_l2_free(mat_mult_l2, buffer_size);

    printf("\nCluster DMA done with %d error(s) !\n", errors);

    pmsis_exit(errors);
}

/* Program Entry. */
int main(void)
{
    printf("\n\n\t *** PMSIS Cluster DMA Test ***\n\n");
    return pmsis_kickoff((void *) MatCompute);
}
