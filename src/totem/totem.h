/**
 * Defines the high-level interface of Totem framework. It offers an interface 
 * to the user of a totem-based algorithm to initialize/finalize the framework's
 * algorithm-agnostic state, and query profiling data recorded during a previous
 * execution. This is basically a wrapper to the engine interface.
 *
 *  Created on: 2012-07-03
 *  Author: Abdullah Gharaibeh
 */

#ifndef TOTEM_H
#define TOTEM_H

#include "totem_comdef.h"
#include "totem_graph.h"
#include "totem_partition.h"

/**
 * Execution platform options
 */
typedef enum {
  PLATFORM_CPU,       // execute on the CPU only
  PLATFORM_GPU,       // execute on GPUs only
  PLATFORM_HYBRID,    // execute on the CPU and the GPUs
  PLATFORM_MAX        // indicates the number of platform options
} platform_t;

/**
 * Partitioning algorithm type
 */
typedef enum {
  PAR_RANDOM = 0,
  PAR_SORTED_ASC,
  PAR_SORTED_DSC,
  PAR_MAX
} partition_algorithm_t;

/**
 * Callback function on a partition to enable algorithm-specific per-partition
 * state allocation/finalization.
 */
typedef void(*totem_cb_func_t)(partition_t*);

/**
 * Defines the attributes used to initialize a Totem
 */
typedef struct totem_attr_s {
  partition_algorithm_t par_algo;      /**< CPU-GPU partitioning strategy */
  platform_t            platform;      /**< the execution platform */
  uint32_t              gpu_count;     /**< number of GPUs to use  */
  gpu_graph_mem_t     gpu_graph_mem;   /**< determines the type of memory used
                                            to place the graph data structure of
                                            GPU partitions */
  bool                  gpu_par_randomized; /**< whether the placement of 
                                                 vertices across GPUs is random
                                                 or according to par_algo */
  float                 cpu_par_share; /**< the percentage of edges assigned
                                            to the CPU partition. Note that this
                                            value is relevant only in hybrid 
                                            platforms. The GPUs will be assigned
                                            equal shares after deducting the CPU
                                            share. If this is set to zero, then
                                            the graph is divided among all 
                                            processors equally. */
  size_t                push_msg_size; /**< push comm. message size in bits */
  size_t                pull_msg_size; /**< pull comm. message size in bits */
  totem_cb_func_t       alloc_func;    /**< callback function to allocate 
                                            application-specific state */
  totem_cb_func_t       free_func;     /**< callback function to free 
                                            application-specific state */
} totem_attr_t;

// default attributes: hybrid (one GPU + CPU) platform, random 50-50 
// partitioning, push message size is word and zero pull message size
#define TOTEM_DEFAULT_ATTR {PAR_RANDOM, PLATFORM_HYBRID, 1, \
      GPU_GRAPH_MEM_DEVICE, false, 0.5, MSG_SIZE_WORD, MSG_SIZE_ZERO, \
      NULL, NULL}

/**
 * Defines the set of timers measured internally by Totem
 */
typedef struct totem_timing_s {
  double engine_init;  /**< Engine initialization  */
  double engine_par;   /**< Partitioning (included in engine_init) */
  double alg_exec;     /**< Algorithm execution alg_(comp + comm) */
  double alg_comp;     /**< Compute phase */
  double alg_comm;     /**< Communication phase (inlcudes scatter/gather) */
  double alg_aggr;     /**< Final result aggregation */
  double alg_scatter;  /**< The scatter step in communication (push mode) */
  double alg_gather;   /**< The gather step in communication (pull mode) */
  double alg_gpu_comp; /**< Computation time of the slowest GPU
                            (included in alg_comp) */
  double alg_gpu_total_comp; /**< Sum of computation time of all GPUs */
  double alg_cpu_comp; /**< CPU computation (included in alg_comp) */
  double alg_init;     /**< Algorithm initialization */
  double alg_finalize; /**< Algorithm finalization */
} totem_timing_t;


/**
 * Initializes the state required for hybrid CPU-GPU processing. It creates a
 * set of partitions equal to the number of GPUs plus one for the CPU. Note that
 * this function initializes algorithm-agnostic state only. This function
 * corresponds to Kernel 1 (the graph construction kernel) of the Graph500 
 * benchmark specification.
 * @param[in] graph  the input graph
 * @param[in] attr   attributes to setup the engine
 */
error_t totem_init(graph_t* graph, totem_attr_t* attr);

/**
 * Clears the state allocated by the engine via the totem_init function.
 */
void totem_finalize();

/**
 * Returns a reference to the set of timers measured internally by the engine
 */
const totem_timing_t* totem_timing();

/**
 * Resets the timers that measure the internals of the engine
 */
void totem_timing_reset();

/**
 * Returns the number of partitions
 */
uint32_t totem_partition_count();

/**
 * Returns the number of vertices in a specific partition
 */
vid_t totem_par_vertex_count(uint32_t pid);

/**
 * Returns the number of edges in a specific partition
 */
eid_t totem_par_edge_count(uint32_t pid);

/**
 * Returns the number of remote vertices in a specific partition
 */
vid_t totem_par_rmt_vertex_count(uint32_t pid);

/**
 * Returns the number of remote edges in a specific partition
 */
eid_t totem_par_rmt_edge_count(uint32_t pid);

#endif  // TOTEM_H
