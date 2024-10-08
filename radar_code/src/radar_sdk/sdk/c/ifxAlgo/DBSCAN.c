/* ===========================================================================
** Copyright (C) 2021 Infineon Technologies AG
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. Neither the name of the copyright holder nor the names of its
**    contributors may be used to endorse or promote products derived from
**    this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** ===========================================================================
*/

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <string.h>

#include "ifxAlgo/DBSCAN.h"

#include "ifxBase/Cube.h"
#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/**
 * @brief Defines the structure for DBSCAN module related settings.
 *        Use type ifx_DBSCAN_t for this struct.
 */
struct ifx_DBSCAN_s
{
    uint16_t min_points;             /**< Minimum number of neighbor points to be recognized as a cluster.*/
    ifx_Float_t min_dist;            /**< Minimum distance at which a point is recognized as a neighbor.*/
    uint16_t max_num_detections;     /**< Maximum number of detections (points) which can appear.*/
    uint8_t* visited;                /**< ... */
    uint8_t* is_noise;               /**< ... */
    uint16_t* neighbors;             /**< ... */
    uint16_t* new_neighbors;         /**< ... */
    ifx_Matrix_R_t* distance_matrix; /**< ... */
};

/*
==============================================================================
   4. LOCAL DATA
==============================================================================
*/

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

static uint8_t exists(uint16_t element,
                      const uint16_t* array,
                      uint16_t num_elements);

static void merge_neighbors(uint16_t* from,
                            uint16_t num_from,
                            uint16_t* to,
                            uint16_t* num_to);

static int check_neighbors(ifx_DBSCAN_t* h,
                           int num_detections,
                           int i,
                           uint16_t* neighbors);

static void dist(const uint16_t* a,
                 uint16_t rows,
                 ifx_Matrix_R_t* distance_matrix);

static void expand_cluster(ifx_DBSCAN_t* h,
                           int num_detections,
                           int detection_idx,
                           uint16_t num_neighbors,
                           uint16_t num_clusters,
                           uint16_t* cluster_vector);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

static uint8_t exists(uint16_t element,
                      const uint16_t* array,
                      uint16_t num_elements)
{
    for (int i = 0; i < num_elements; i++)
    {
        if (array[i] == element)
        {
            return 1;
        }
    }

    return 0;
}

//----------------------------------------------------------------------------

static void merge_neighbors(uint16_t* from,
                            uint16_t num_from,
                            uint16_t* to,
                            uint16_t* num_to)
{
    for (uint16_t i = 0; i < num_from; i++)
    {
        if (!exists(from[i], to, *num_to))
        {
            to[*num_to] = from[i];
            (*num_to)++;
        }
    }
}

//----------------------------------------------------------------------------

static int check_neighbors(ifx_DBSCAN_t* h,
                           int num_detections,
                           int i,
                           uint16_t* neighbors)
{
    int n = 0;

    for (int j = 0; j < num_detections; j++)
        if (mAt(h->distance_matrix, i, j) <= h->min_dist)
        {
            neighbors[n++] = j;
        }

    return n;
}

//----------------------------------------------------------------------------

static void dist(const uint16_t* a,
                 uint16_t rows,
                 ifx_Matrix_R_t* distance_matrix)
{
    for (int i = 0; i < rows; i++)
    {
        ifx_Float_t x1 = a[i * 2];
        ifx_Float_t y1 = a[i * 2 + 1];

        for (int j = 0; j < rows; j++)
        {
            ifx_Float_t x2 = a[j * 2];
            ifx_Float_t y2 = a[j * 2 + 1];

            mAt(distance_matrix, i, j) = HYPOT(x2 - x1, y2 - y1);
        }
    }
}

//----------------------------------------------------------------------------

static void expand_cluster(ifx_DBSCAN_t* h,
                           int num_detections,
                           int detection_idx,
                           uint16_t num_neighbors,
                           uint16_t num_clusters,
                           uint16_t* cluster_vector)
{
    cluster_vector[detection_idx] = num_clusters;

    for (int n_i = 0; n_i < num_neighbors; n_i++)
    {
        int cur_det_i = h->neighbors[n_i];

        if (!h->visited[cur_det_i])
        {
            h->visited[cur_det_i] = 1;
            int num_new_neighbors = check_neighbors(h, num_detections, cur_det_i, h->new_neighbors);

            if (num_new_neighbors >= h->min_points)
            {
                merge_neighbors(h->new_neighbors, num_new_neighbors, h->neighbors, &num_neighbors);
            }
        }

        if (cluster_vector[cur_det_i] == 0)
        {
            cluster_vector[cur_det_i] = num_clusters;
        }
    }
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_DBSCAN_t* ifx_dbscan_create(const ifx_DBSCAN_Config_t* config)
{
    ifx_DBSCAN_t* h = NULL;

    IFX_ERR_BRN_NULL(config);
    IFX_ERR_BRN_ARGUMENT(config->min_points < 1);
    IFX_ERR_BRN_ARGUMENT(config->min_dist <= 0);
    IFX_ERR_BRN_ARGUMENT(config->max_num_detections <= config->min_points);

    h = ifx_mem_calloc(1, sizeof(struct ifx_DBSCAN_s));
    IFX_ERR_BRN_MEMALLOC(h);

    h->max_num_detections = config->max_num_detections;
    h->min_dist = config->min_dist;
    h->min_points = config->min_points;

    h->distance_matrix = ifx_mat_create_r(config->max_num_detections, config->max_num_detections);
    h->is_noise = ifx_mem_calloc(config->max_num_detections, sizeof(uint8_t));
    h->visited = ifx_mem_calloc(config->max_num_detections, sizeof(uint8_t));
    h->neighbors = ifx_mem_calloc(config->max_num_detections, sizeof(uint16_t));
    h->new_neighbors = ifx_mem_calloc(config->max_num_detections, sizeof(uint16_t));

    if (h->distance_matrix == NULL
        || h->is_noise == NULL
        || h->visited == NULL
        || h->neighbors == NULL
        || h->new_neighbors == NULL)
    {
        ifx_dbscan_destroy(h);
        IFX_ERR_BRN_MEMALLOC(NULL);
    }

    return h;
}

//----------------------------------------------------------------------------

void ifx_dbscan_destroy(ifx_DBSCAN_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_mat_destroy_r(handle->distance_matrix);
    ifx_mem_free(handle->is_noise);
    ifx_mem_free(handle->visited);
    ifx_mem_free(handle->neighbors);
    ifx_mem_free(handle->new_neighbors);

    ifx_mem_free(handle);
}

//----------------------------------------------------------------------------

void ifx_dbscan_run(ifx_DBSCAN_t* handle,
                    const uint16_t* detections,
                    uint16_t num_detections,
                    uint16_t* cluster_vector)
{
    int num_clusters = 0;

    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NULL(detections);
    IFX_ERR_BRK_NULL(cluster_vector);

    IFX_ERR_BRK_ARGUMENT(num_detections > handle->max_num_detections);

    memset(cluster_vector, 0, num_detections * sizeof(uint16_t));
    memset(handle->is_noise, 0, handle->max_num_detections);
    memset(handle->visited, 0, handle->max_num_detections);

    dist(detections, num_detections, handle->distance_matrix);

    for (int i = 0; i < num_detections; i++)
    {
        if (!handle->visited[i])
        {
            handle->visited[i] = 1;
            int num_neighbors = check_neighbors(handle, num_detections, i, handle->neighbors);

            if (num_neighbors < handle->min_points)
            {
                handle->is_noise[i] = 1;
            }
            else
            {
                num_clusters++;
                expand_cluster(handle, num_detections, i, num_neighbors, num_clusters, cluster_vector);
            }
        }
    }
}

//----------------------------------------------------------------------------

void ifx_dbscan_set_min_points(ifx_DBSCAN_t* handle,
                               uint16_t min_points)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_ARGUMENT(min_points < 1);

    handle->min_points = min_points;
}

//----------------------------------------------------------------------------

void ifx_dbscan_set_min_distance(ifx_DBSCAN_t* handle,
                                 ifx_Float_t min_distance)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_ARGUMENT(min_distance <= 0);

    handle->min_dist = min_distance;
}
