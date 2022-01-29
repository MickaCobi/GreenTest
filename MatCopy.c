#include "pmsis.h"

// Matrix properties definition
#define MATRIX_WIDTH 	(64)
#define MATRIX_HEIGHT 	(64)
#define MATRIX_SIZE		(MATRIX_WIDTH * MATRIX_HEIGHT)

static unsigned short *p_mat1_l2_buf; 		// Matrix1 Buffer pointer in L2 memory
static unsigned short *p_mat2_l2_buf; 		// Matrix2 Buffer pointer in L2 memory
static unsigned short *p_mat1_fc_l1_buf; 	// Matrix1 Buffer pointer in FC L1 memory
static unsigned short *p_mat2_fc_l1_buf; 	// Matrix2 Buffer pointer in FC L1 memory
static struct pi_device dmacpy;
static int errors = 0;

static void __L2_FCL1_copy_end(void* arg)
{
	dma_memcpy_t *copy = (dma_memcpy_t *) arg;
    dma_memcpy_t *next = copy->next;
    if (next != NULL) // Here, there are still DMA transfer to perform
    {
    	// Create callback for next transfer
    	pi_task_callback(next->task, __L2_FCL1_copy_end, copy->next);

    	// Initiate next transfer
        errors = pi_dmacpy_copy_async(&dmacpy, (void *) next->src, (void *) next->dst,
                                      next->size, next->dir, next->task);
        if(errors)
        {
            printf("Copy from %p to %p failed : %ld\n", next->src, next->dst, errors);
        }
        else
        {
        	printf("Copy from %p to %p succeed : %ld\n", next->src, next->dst, errors);
        }
    }
    else // Here, matrix transfer from L2 to FC L1 is finished
    {
    	// Proceed to calculation
    }
}


void mainController(void)
{
    struct pi_dmacpy_conf 	dmacpy_conf;
    pi_task_t task_copy_mat1_l2_fcl1; 	// Task dedicated to copying Matrix1 from L2 to L1
    pi_task_t task_copy_mat2_l2_fcl1; 	// Task dedicated to copying Matrix2 from L2 to L1	
	dma_memcpy_t dma_copy_mat1_l2_fcl1;	// DMA transfer about Matrix1 from L2 to L1
	dma_memcpy_t dma_copy_mat2_l2_fcl1; // DMA transfer about Matrix2 from L2 to L1

	// Memory allocation

	// Matrix one allocation in L2
	p_mat1_l2_buf = (unsigned short*)pi_l2_malloc(MATRIX_SIZE);
    if(p_mat1_l2_buf == NULL)
    {
        printf("Failed to allocate Matrix1 in L2 !\n");
        pmsis_exit(-1);
    }
    else
    {
    	printf("Matrix1 allocated in L2 at address %p\n", p_mat1_l2_buf);
    }

    // Matrix two allocation in L2
	p_mat2_l2_buf = (unsigned short*)pi_l2_malloc(MATRIX_SIZE);
    if(p_mat2_l2_buf == NULL)
    {
        printf("Failed to allocate Matrix2 in L2!\n");
        pmsis_exit(-1);
    }
    else
    {
    	printf("Matrix2 allocated in L2 at address %p\n", p_mat2_l2_buf);
    }

    // Matrix one allocation in FC L1
	p_mat1_fc_l1_buf = (unsigned short*)pi_fc_l1_malloc(MATRIX_SIZE);
    if(p_mat1_fc_l1_buf == NULL)
    {
        printf("Failed to allocate Matrix1 in FC L1 !\n");
        pmsis_exit(-1);
    }
    else
    {
    	printf("Matrix1 allocated in FC L1 at address %p\n", p_mat1_fc_l1_buf);
    }

    // Matrix two allocation in L2
	p_mat2_cl_l2_buf = (unsigned short*)pi_fc_l1_malloc(MATRIX_SIZE);
    if(p_mat2_cl_l2_buf == NULL)
    {
        printf("Failed to allocate Matrix2 in CL L1!\n");
        pmsis_exit(-1);
    }
    else
    {
    	printf("Matrix2 allocated in FC L1 at address %p\n", p_mat2_cl_l2_buf);
    }

    // FILL L2 MEMORY 

    for(int id = 0; id < MATRIX_SIZE; id++)
    {
    	p_mat1_l2_buf[id] = 2;
    	p_mat2_l2_buf[id] = 3;
    	p_mat1_fc_l1_buf[id] = 0;
    	p_mat2_fc_l2_buf[id] = 0;
    }

    // CONFIGURE DMA TRANSFERS

    pi_dmacpy_conf_init(&dmacpy_conf);
    pi_open_from_conf(&dmacpy, &dmacpy_conf);
    errors = pi_dmacpy_open(&dmacpy);
    if (errors)
    {
        printf("Error dmacpy open : %ld !\n", errors);
        pmsis_exit(-3);
    }

    // Transfer 1 : Matrix 1 from L2 to FC L1
    dma_copy_mat1_l2_fcl1.src 	= p_mat1_l2_buf;
    dma_copy_mat1_l2_fcl1.dst 	= p_mat1_fc_l1_buf;
    dma_copy_mat1_l2_fcl1.size 	= MATRIX_SIZE * sizeof(unsigned short);
    dma_copy_mat1_l2_fcl1.dir 	= PI_DMACPY_L2_FC_L1; 
    dma_copy_mat1_l2_fcl1.task 	= &task_copy_mat1_l2_fcl1;
    dma_copy_mat1_l2_fcl1.next 	= &task_copy_mat2_l2_fcl1;

    // Transfer 2 : Matrix 2 from L2 to FC L1
    dma_copy_mat1_l2_fcl1.src 	= p_mat1_l2_buf;
    dma_copy_mat1_l2_fcl1.dst 	= p_mat1_fc_l1_buf;
    dma_copy_mat1_l2_fcl1.size 	= MATRIX_SIZE * sizeof(unsigned short);
    dma_copy_mat1_l2_fcl1.dir 	= PI_DMACPY_L2_FC_L1; 
    dma_copy_mat1_l2_fcl1.task 	= &task_copy_mat1_l2_fcl1;
    dma_copy_mat1_l2_fcl1.next 	= NULL; 

    // Configure callbacks 
    // Calling "__L2_FCL1_copy_end" function when all matrix has been copied to FC L1 memory.
    pi_task_callback(&task_copy_mat1_l2_fcl1, __L2_FCL1_copy_end, &arg);


    errors = pi_dmacpy_copy_async(&dmacpy, (void *)p_mat1_l2_buf, (void *)p_mat1_fc_l1_buf,
                                  (MATRIX_SIZE * sizeof(unsigned short)), PI_DMACPY_L2_FC_L1,
                                  &task_copy);
    if(errors)
    {
        printf("Copy from %p to %p failed : %ld\n", p_mat1_l2_buf, p_mat1_fc_l1_buf, errors);
    }
    else
    {
    	printf("Copy from %p to %p succeed : %ld\n", p_mat1_l2_buf, p_mat1_fc_l1_buf, errors);
    }
    pmsis_exit(errors);
}

int main(void)
{
	return pmsis_kickoff((void*)mainController);
}