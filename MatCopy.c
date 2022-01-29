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

void mainController(void)
{
	int errors = 0;
    struct pi_dmacpy_conf 	dmacpy_conf;
    pi_task_t task_copy_mat1_l2_fcl1; // Task dedicated to copying Matrix1 from L2 to L1
    pi_task_t task_copy_mat2_l2_fcl1; // Task dedicated to copying Matrix2 from L2 to L1	
	dma_memcpy_t

	// Memory allocation

	// Matrix one allocation in L2
	p_mat1_l2_buf = (unsigned short*)pi_l2_malloc(MATRIX_SIZE);
    if(p_mat1_l2_buf == NULL)
    {
        printf("Failed to allocate Matrix1 in L2 !\n");
        pmsis_exit(-1);
    }

    // Matrix two allocation in L2
	p_mat2_l2_buf = (unsigned short*)pi_l2_malloc(MATRIX_SIZE);
    if(p_mat2_l2_buf == NULL)
    {
        printf("Failed to allocate Matrix2 in L2!\n");
        pmsis_exit(-1);
    }

    // Matrix one allocation in FC L1
	p_mat1_fc_l1_buf = (unsigned short*)pi_fc_l1_malloc(MATRIX_SIZE);
    if(p_mat1_fc_l1_buf == NULL)
    {
        printf("Failed to allocate Matrix1 in FC L1 !\n");
        pmsis_exit(-1);
    }

    // Matrix two allocation in L2
	p_mat2_cl_l1_buf = (unsigned short*)pi_fc_l1_malloc(MATRIX_SIZE);
    if(p_mat2_fc_l1_buf == NULL)
    {
        printf("Failed to allocate Matrix2 in CL L1!\n");
        pmsis_exit(-1);
    }

    // FILL BUFFERS 

    pi_dmacpy_conf_init(&dmacpy_conf);
    pi_open_from_conf(&dmacpy, &dmacpy_conf);
    errors = pi_dmacpy_open(&dmacpy);
    if (errors)
    {
        printf("Error dmacpy open : %ld !\n", errors);
        pmsis_exit(-3);
    }

    for(int id = 0; id < MATRIX_SIZE; id++)
    {
    	p_mat1_l2_buf[id] = 2;
    	p_mat2_l2_buf[id] = 3;
    	p_mat1_fc_l1_buf[id] = 0;
    	p_mat2_fc_l2_buf[id] = 0;
    }



    pmsis_exit(errors);
}

int main(void)
{
	return pmsis_kickoff((void*)mainController);
}