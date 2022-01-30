#include "pmsis.h"

// Matrix properties definition
#define MATRIX_WIDTH 	(16)
#define MATRIX_HEIGHT 	(16)
#define MATRIX_SIZE		(MATRIX_WIDTH * MATRIX_HEIGHT)

static unsigned short *p_mat1_l1_buf; 		// Matrix1 Buffer pointer in L1 memory
static unsigned short *p_mat2_l1_buf; 		// Matrix2 Buffer pointer in L1 memory
static unsigned short *p_mat1_l2_buf; 		// Matrix1 Buffer pointer in L2 memory
static unsigned short *p_mat2_l2_buf; 		// Matrix2 Buffer pointer in L2 memory

static unsigned short *p_mat_add_l1_buf; 		// Matrix add Buffer pointer in L1 memory
static unsigned short *p_mat_mult_l1_buf; 		// Matrix mult Buffer pointer in L1 memory
static unsigned short *p_mat_add_l2_buf; 		// Matrix add Buffer pointer in L2 memory
static unsigned short *p_mat_mult_l2_buf; 		// Matrix mult Buffer pointer in L2 memory


static struct pi_cluster_task task_add;		// task dedicated to add the two matrix
static struct pi_cluster_task task_mult;	// task dedicated to multiply the two matrix


void addMatrix(void* arg)
{
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		p_mat_add_l1_buf[i] = p_mat1_l1_buf[i] + p_mat2_l1_buf[i];
	}
	printf("add task done\n");
}

void multMatrix(void* arg)
{
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		p_mat_mult_l1_buf[i] = p_mat1_l1_buf[i] * p_mat2_l1_buf[i];
	}
	printf("mult task done\n");
}


void mainController(void)
{
	int errors = 0;
    struct pi_dmacpy_conf 	dmacpy_conf;
	struct pi_device 		device;
    struct pi_device 		cluster_dev;

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
	p_mat1_l1_buf = (unsigned short*)pi_fc_l1_malloc(MATRIX_SIZE);
    if(p_mat1_l1_buf == NULL)
    {
        printf("Failed to allocate Matrix1 in L1 !\n");
        pmsis_exit(-1);
    }
    else
    {
    	printf("Matrix1 allocated in L1 at address %p\n", p_mat1_l1_buf);
    }

    // Matrix two allocation in FC L1
	p_mat2_l1_buf = (unsigned short*)pi_fc_l1_malloc(MATRIX_SIZE);
    if(p_mat2_l1_buf == NULL)
    {
        printf("Failed to allocate Matrix2 in L1!\n");
        pmsis_exit(-1);
    }
    else
    {
    	printf("Matrix2 allocated in L1 at address %p\n", p_mat2_l1_buf);
    }

    // FILL L2 MEMORY 

    for(int id = 0; id < MATRIX_SIZE; id++)
    {
    	p_mat1_l2_buf[id] = 2;
    	p_mat2_l2_buf[id] = 3;
    	p_mat1_l1_buf[id] = 0;
    	p_mat2_l1_buf[id] = 0;
    }

    // CONFIGURE DMA TRANSFERS

    pi_dmacpy_conf_init(&dmacpy_conf);
    pi_open_from_conf(&device, &dmacpy_conf);
    errors = pi_dmacpy_open(&device);
    if (errors)
    {
        printf("Error device open : %ld !\n", errors);
        pmsis_exit(-3);
    }


    // Copy Matrix1 from L2 to L1.
    errors = pi_dmacpy_copy(&device, (void *)p_mat1_l2_buf, (void *)p_mat1_l1_buf,
                            (MATRIX_SIZE * sizeof(unsigned short)), PI_DMACPY_L2_FC_L1);
    if (errors)
    {
    	printf("Copy from %p to %p failed : %ld\n", p_mat1_l2_buf, p_mat1_l1_buf, errors);
        pmsis_exit(-4);
    }
    printf("Copy from %p to %p done.\n", p_mat1_l2_buf, p_mat1_l1_buf);


    // Copy Matrix2 from L2 to L1.
    errors = pi_dmacpy_copy(&device, (void *)p_mat2_l2_buf, (void *)p_mat2_l1_buf,
                            (MATRIX_SIZE * sizeof(unsigned short)), PI_DMACPY_L2_FC_L1);
    if (errors)
    {
    	printf("Copy from %p to %p failed : %ld\n", p_mat2_l2_buf, p_mat2_l1_buf, errors);
        pmsis_exit(-4);
    }
    printf("Copy from %p to %p done.\n", p_mat2_l2_buf, p_mat2_l1_buf);


    // Start a task to compute add matrix
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&task_add, addMatrix, NULL));

    // Start a task to compute MultMatrix
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&task_mult, multMatrix, NULL));


    pmsis_exit(errors);
}

int main(void)
{
	return pmsis_kickoff((void*)mainController);
}