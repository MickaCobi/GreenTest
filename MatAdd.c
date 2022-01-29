#include <stdio.h>
#include "pmsis.h"
#include "Gap.h"
#include "MatAddKernels.h"

#define MATRIX_WIDTH 	(64)
#define MATRIX_HEIGHT 	(64)
#define MATRIX_SIZE		(MATRIX_WIDTH * MATRIX_HEIGHT)

extern char *L1_Memory;

PI_L2 int Matrix1[MATRIX_SIZE]; // Todo: Meaning of PI_L2 ? 
PI_L2 int Matrix2[MATRIX_SIZE];
PI_L2 int Matrix3[MATRIX_SIZE];

static void cluster_add(void)
{
	MatADD(Matrix1, Matrix2, Matrix3); // Todo : How the data is computed here
										// How the data is copied to L1?
}

void addMatrix(void)
{
	int errors = 0;
	struct pi_cluster_conf 	cluster_conf;
	struct pi_device 		device;
	struct pi_cluster_task 	cluster_task;

	// Initialize matrix
	for(int i = 0; i < MATRIX_SIZE; i++)
    {
        Matrix1[i] = 2;
        Matrix2[i] = 2;
        Matrix3[i] = 0;
    }

	// Get default configuration
	pi_cluster_conf_init(&cluster_conf);
	cluster_conf.id = (uint32_t)0;
	pi_open_from_conf(&device,&cluster_conf);
	if(pi_cluster_open(&device))
		pmsis_exit(-1);

	L1_Memory = (char *) pi_l1_malloc(&device, _L1_Memory_SIZE);
    if (L1_Memory == 0 )
    {
        pmsis_exit(-2);
    }

	cluster_task.entry = cluster_add; // fonction to be exec by the task
	cluster_task.arg = NULL;
	cluster_task.stack_size = (uint32_t)1024; // Todo: meaning a the stack size ? 

    pi_cluster_send_task_to_cl(&device, &cluster_task);

    pi_l1_free(&device, L1_Memory, _L1_Memory_SIZE); // Free L1 memory

	pi_cluster_close(&device);

	for (int i = 0; i < MATRIX_HEIGHT; i++)
    {
        for (int j = 0; j < MATRIX_WIDTH; j++)
        {
            if (Matrix3[(i * MATRIX_WIDTH) + j] != 4)
            {
                errors++;
            }
        }
    }
    printf("Test %s with %ld error(s) !\n", (errors) ? "failed" : "success", errors);
    pmsis_exit(errors);
}

int main(void)
{
	return pmsis_kickoff((void*)addMatrix);
}