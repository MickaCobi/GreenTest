#include "pmsis.h"

// Matrix properties definition
#define MATRIX_WIDTH 	(64)
#define MATRIX_HEIGHT 	(64)
#define MATRIX_SIZE		(MATRIX_WIDTH * MATRIX_HEIGHT)

static unsigned short *p_mat1_l2_buf, *p_mat2_l2_buf, *p_mat1_cl_l1_buf, *p_mat2_cl_l1_buf;

void mainController(void)
{
	int errors = 0;
	struct pi_cluster_conf 	cluster_conf;
	struct pi_device 		device; // global ? 
	struct pi_cluster_task 	cluster_task;

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

    // Matrix one allocation in CL L1
	p_mat1_cl_l1_buf = (unsigned short*)pi_cl_l1_malloc(&device,(uint32_t)MATRIX_SIZE);
    if(p_mat1_cl_l1_buf == NULL)
    {
        printf("Failed to allocate Matrix1 in CL L1 !\n");
        pmsis_exit(-1);
    }

    // Matrix two allocation in L2
	p_mat2_cl_l1_buf = (unsigned short*)pi_cl_l1_malloc(&device, (uint32_t)MATRIX_SIZE);
    if(p_mat2_cl_l1_buf == NULL)
    {
        printf("Failed to allocate Matrix2 in CL L1!\n");
        pmsis_exit(-1);
    }


    pmsis_exit(errors);
}

int main(void)
{
	return pmsis_kickoff((void*)mainController);
}