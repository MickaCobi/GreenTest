#include "pmsis.h"

// Matrix properties definition
#define MATRIX_WIDTH 	(64)
#define MATRIX_HEIGHT 	(64)
#define MATRIX_SIZE		(MATRIX_WIDTH * MATRIX_HEIGHT)

static unsigned short *p_mat1_l2_buf, *p_mat2_l2_buf, *p_mat1_fc_l1_buf, *p_mat2_fc_l1_buf;
static struct pi_device dmacpy;

void mainController(void)
{
	int errors = 0;
	struct pi_cluster_conf 	cluster_conf;
	struct pi_device 		device; // global ? 
	struct pi_cluster_task 	cluster_task;
    struct pi_dmacpy_conf 	dmacpy_conf;

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