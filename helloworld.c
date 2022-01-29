#include "pmsis.h"

void helloTask(void* arg)
{
	printf("hello world [%d,%d]\n",pi_cluster_id(),pi_core_id());
}

void clusterEntryPoint(void* arg)
{
	int nb_cores = pi_cl_cluster_nb_cores();
	pi_cl_team_fork(nb_cores, helloTask, arg);
}

void sayHello(void)
{
	int err = 0;

	// Get ids
	uint32_t core_id = pi_core_id(), cluster_id = pi_cluster_id();
	
	struct pi_cluster_conf 	cluster_conf;
	struct pi_device 		device;
	struct pi_cluster_task 	cluster_task;

	// Printing first core
	printf("hello world [%d,%d]\n",pi_cluster_id(),pi_core_id());

	// Get default configuration
	pi_cluster_conf_init(&cluster_conf);

	pi_open_from_conf(&device,&cluster_conf);
	if(pi_cluster_open(&device))
		pmsis_exit(-1);

	// Initialize & send cluster task
	pi_cluster_send_task(&device, pi_cluster_task(&cluster_task, clusterEntryPoint, NULL));
	pi_cluster_close(&device);
	pmsis_exit(err);
}

int main(void)
{
	return pmsis_kickoff((void*)sayHello);
}