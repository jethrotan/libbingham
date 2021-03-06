#ifndef BINGHAM_OLF_H
#define BINGHAM_OLF_H


#include "bingham.h"
#include "hll.h"

#include <flann/flann.h>

#include <stdio.h>

#include "cuda_wrapper.h"

/*#ifdef __cplusplus
extern "C" {
#endif*/

  // point cloud data structure
  typedef struct {
    int num_channels;
    int num_points;
    char **channels;  // e.g. {"x", "y", "z"}
    double **data;    // e.g. data[0] = {x1,x2,x3}, data[1] = {y1,y2,y3}, etc.

    // data pointers
    double *pc1;
    double *pc2;
    double *range_edge;
    double *curv_edge;
    double *img_edge;
    double *normalvar;
    double *canny;

    // transposed data
    double **points;
    double **views;
    double **colors;
    double **normals;
    double **principal_curvatures;
    double **fpfh;
    double **shot;
    double **sift;
    double **ved;
    //double **sdw;
    double **labdist;

    // computed data
    int *clusters;
    double **quaternions; //[2]
    double **lab;

    // data sizes
    int fpfh_length;
    int shot_length;
    int sift_length;
    int ved_length;
    //int sdw_length;
    int labdist_length;

    //kdtree_t *points_kdtree;
    //pcd_balls_t *balls;

  } pcd_t;


  typedef struct {
    double **lab;
    double **means[2];
    double ***covs[2];
    int *cnts[2];
    int num_points;
    double **avg_cov;
  } pcd_color_model_t;


  typedef struct {
    double *x;
    double *q;
    bingham_t *B;
  } olf_t;


  // range image
  typedef struct {
    double res;
    double min[2];
    double vp[7];
    int w;
    int h;
    double **image;  // stored column-wise: image[x][y]
    int **idx;
    double ***points;      // average point in each cell
    double ***normals;     // average normal in each cell
    double ***lab_colors;  // average LAB color in each cell
    int **cnt;             // number of points in each cell
  } range_image_t;


  typedef struct {
    int nx;
    int ny;
    int nz;
    double res;
    double min[3];
    double *nn_dist;  // flattened NN distances
    int *nn_cell;     // flattened NN grid indices
    int **pcd_idx;    // flattened pcd indices (up to 2 points per cell)
    pcd_t *pcd;
  } dist_grid_t;


#define MAX_SYMMETRY_PARAM_LENGTH 12  // three planes

#define PLANE_SYMMETRY 1
#define LINE_SYMMETRY 2
#define POINT_SYMMETRY 3
#define DUAL_PLANE_SYMMETRY 4
#define TRIPLE_PLANE_SYMMETRY 5

  typedef struct {
    int n;
    int *types;
    double **params;
    double *err;
  } symmetries_t;


  // logistic regression coefficients
  typedef struct score_comp_models_struct {
    double b_xyz[2];
    double b_normal[2];
    //double b_vis[2];
    double b_random_walk[2];
    double b_edge[2];
    //double b_edge_vis[2];
    double b_edge_occ[2];
    double b_color_L[2];
    double b_color_A[2];
    double b_color_B[2];
    //double b_specularity[2];
    double b_fpfh[2];
    //double b_labdist[2];
    //double b_segment_affinity[2];
  } score_comp_models_t;


  typedef struct {
    int center_point;
    int *surface_points;
    int *edge_points;
    double *edge_weights;
    int num_surface_points;
    int num_edge_points;
    double max_radius;
    double avg_point[3];
    double avg_normal[3];
    double avg_lab_color[3];
    int num_pixels;
  } superpixel_t;


  typedef struct {
    pcd_t *pcd;      // must be sorted by viewpoint!
    int num_views;
    double **views;  // Nx3 matrix of the unique viewpoints
    int *view_idx;   // view_idx[i] = index of first pcd point with views[i] as its viewpoint
    int *view_cnt;   // view_cnt[i] = number of pcd points with views[i] as their viewpoint
  } multiview_pcd_t;


  typedef struct {
    double X[3];
    double Q[4];
  } simple_pose_t;


  // BPA (Bingham Procrustean Alignment)

  typedef struct {
    char *name;
    pcd_t *obj_pcd;
    pcd_t *fpfh_pcd;
    pcd_t *shot_pcd;
    pcd_t *sift_pcd;
    pcd_t *range_edges_pcd;
    dist_grid_t *dist_grid;
    symmetries_t *symmetries;
    score_comp_models_t *score_comp_models;
  } olf_model_t;


  typedef struct {
    pcd_t *bg_pcd;
    pcd_t *fpfh_pcd;
    pcd_t *shot_pcd;
    pcd_t *sift_pcd;
    double *table_plane;
  } olf_obs_t;


  typedef struct {
    double range_sigma;
    double normal_sigma;
    double lab_sigma[3];
  } scope_noise_model_t;


  typedef struct scope_params_struct {  // scope_params_t

    // GENERAL PARAMS
    int verbose;
    int use_true_pose;
    int add_true_pose_x_noise;
    int add_true_pose_q_noise;
    int use_fpfh;
    int use_shot;
    int use_sift;
    int use_table;
    int use_colors;
    int num_samples_round1;
    int num_samples_round2;
    int num_samples_round3;
    int num_correspondences;
    int branching_factor;
    int num_validation_points;
    int knn;
    int round2_alignment_iter;
    int final_alignment_iter;
    int use_cuda;
    int align_model_to_segments_iter;
    int use_bpa;
    int test_bpa;

    // WEIGHT / DISTANCE PARAMS
    int dispersion_weight;
    double sift_dthresh;
    double vis_thresh;
    double round1_range_thresh;
    double round1_score_thresh;
    double round4_score_thresh;
    int xyz_score_window;
    int xyz_score_use_plane;

    // NOISE MODEL PARAMS
    double xyz_sigma;
    double range_sigma;
    double normal_sigma;
    double normalvar_thresh;
    double lab_sigma;
    double f_sigma;
    double shot_sigma;

    // SCORE2 PARAMS
    int score2_use_score_comp_models;
    double score2_xyz_weight;
    double score2_normal_weight;
    double score2_vis_weight;
    double score2_random_walk_weight;
    double score2_edge_weight;
    double score2_edge_vis_weight;
    double score2_edge_occ_weight;
    double score2_L_weight;
    double score2_A_weight;
    double score2_B_weight;
    double score2_fpfh_weight;
    double score2_specularity_weight;
    double score2_segment_affinity_weight;
    double score2_segment_weight;
    double score2_table_weight;

    // SCORE3 PARAMS
    int score3_use_score_comp_models;
    double score3_xyz_weight;
    double score3_normal_weight;
    double score3_vis_weight;
    double score3_random_walk_weight;
    double score3_edge_weight;
    double score3_edge_vis_weight;
    double score3_edge_occ_weight;
    double score3_L_weight;
    double score3_A_weight;
    double score3_B_weight;
    double score3_fpfh_weight;
    double score3_specularity_weight;
    double score3_segment_affinity_weight;
    double score3_segment_weight;
    double score3_table_weight;

    // POSE CLUSTERING PARAMS
    int pose_clustering;
    double x_cluster_thresh;
    double q_cluster_thresh;

    // IMAGE PARAMS
    double range_image_resolution;
    int use_fg_edge_image;
    double range_edge_weight;
    double curv_edge_weight;
    double img_edge_weight;
    int edge_blur;
    int color_blur;
    double min_edge_prob;
    int segment_resolution;

    //double surfdist_weight;
    //double surfwidth_weight;
    //double surfdist_thresh;
    //double surfwidth_thresh;
    //double surfdist_sigma;
    //double surfwidth_sigma;
    //double fsurf_sigma;

    // MOPE PARAMS
    /*double mope_r1_scope_score_weight;
    double mope_r1_unexplained_weight;
    double mope_r1_overlap_weight;    
    double mope_r1_overlap_per_object_weight;
    double mope_r1_num_taken_weight;

    double mope_r2_scope_score_weight;
    double mope_r2_unexplained_weight;
    double mope_r2_overlap_weight;    
    double mope_r2_overlap_per_object_weight;
    double mope_r2_num_taken_weight;
    
    int score_comp_models;
    int num_rounds;

    int plot_true; //dbug*/

  } scope_params_t;

  typedef struct mope_params_struct {  // mope_params_t

    // SCOPE PARAMS
    double scope_xyz_weight;
    double scope_normal_weight;
    double scope_vis_weight;
    double scope_random_walk_weight;
    double scope_edge_weight;
    double scope_edge_vis_weight;
    double scope_edge_occ_weight;
    double scope_L_weight;
    double scope_A_weight;
    double scope_B_weight;
    double scope_fpfh_weight;
    double scope_specularity_weight;
    double scope_segment_affinity_weight;
    double scope_segment_weight;
    double scope_table_weight;

    // MOPE PARAMS
    double round1_scope_score_weight;
    double round1_unexplained_weight;
    double round1_overlap_weight;    
    double round1_overlap_per_object_weight;
    double round1_num_taken_weight;

    double round2_scope_score_weight;
    double round2_unexplained_weight;
    double round2_overlap_weight;    
    double round2_overlap_per_object_weight;
    double round2_num_taken_weight;
    
    int score_comp_models;
    int num_rounds;

    int plot_true; //dbug

  } mope_params_t;

  typedef struct scope_model_data_struct {
    pcd_t *pcd_model;
    pcd_t *fpfh_model;
    pcd_t *shot_model;
    pcd_t *sift_model;
    symmetries_t *model_symmetries;
    score_comp_models_t *score_comp_models;
    olf_t *pcd_model_olfs;
    olf_t *fpfh_model_olfs;
    olf_t *shot_model_olfs;
    olf_t *sift_model_olfs;
    olf_t *range_edges_model_olfs;
    int *model_to_fpfh_map;
    int *model_to_shot_map;
    pcd_color_model_t *color_model;
    multiview_pcd_t *range_edges_model;
    dist_grid_t *model_dist_grid;
    double *fpfh_model_cmf;
    double *shot_model_cmf;
    struct FLANNParameters model_xyz_params;
    struct FLANNParameters model_xyzn_params;
    struct FLANNParameters fpfh_model_f_params;
    struct FLANNParameters fpfh_model_xyzn_params;
    struct FLANNParameters shot_model_f_params;
    struct FLANNParameters shot_model_xyzn_params;
    flann_index_t model_xyz_index;
    flann_index_t model_xyzn_index;
    flann_index_t fpfh_model_f_index;
    flann_index_t fpfh_model_xyzn_index;
    flann_index_t shot_model_f_index;
    flann_index_t shot_model_xyzn_index;
  } scope_model_data_t;


  typedef struct scope_obs_data_struct {
    pcd_t *pcd_obs;
    pcd_t *fpfh_obs;
    pcd_t *shot_obs;
    pcd_t *sift_obs;
    double *table_plane;
    olf_t *pcd_obs_olfs;
    olf_t *fpfh_obs_olfs;
    olf_t *shot_obs_olfs;
    olf_t *sift_obs_olfs;
    int *obs_to_shot_map;
    range_image_t *obs_range_image;
    range_image_t *obs_fg_range_image;
    int num_obs_edge_points;
    int *obs_edge_idx;
    double **obs_edge_points;
    double **obs_edge_points_image;
    double **obs_edge_image;
    double ***obs_lab_image;
    double **obs_canny_image;
    superpixel_t *obs_segments;
    int **obs_segment_image;
    double **obs_segment_affinities;
    int num_obs_segments;
    struct FLANNParameters fpfh_obs_xyzn_params;
    struct FLANNParameters shot_obs_xyzn_params;
    flann_index_t fpfh_obs_xyzn_index;
    flann_index_t shot_obs_xyzn_index;
  } scope_obs_data_t;



  enum {C_TYPE_FPFH, C_TYPE_SHOT, C_TYPE_SIFT, C_TYPE_EDGE, C_TYPE_SURFACE};


  typedef struct {
    // A star needs to be added back to each of these except n
    int n;

    //dbug
    double *vis_probs;
    double *xyz_dists;
    double *fpfh_dists;
    double *normal_dists;
    int num_range_edge_points;
    int **range_edge_pixels;
    double **range_edge_points;
  } olf_pose_sample_t;

  typedef struct scope_sample_struct {
    double x[3];
    double q[4];
    bingham_t B;
    double x0[3];  // rotate by B about x0, then shift by x (only valid after round 4)
    int *c_obs;
    int *c_model;
    int *c_type;
    double *c_score;
    int nc;
    //olf_t *obs_olfs;
    //olf_t *model_olfs;

    // current superpixel segmentation
    int num_segments;
    int *segments_idx;
    double *segment_probs;

    // validation memory
    int num_edge_outliers;
    int *edge_outliers_idx;
    int num_xyz_outliers;
    int *xyz_outliers_idx;

    //dbug
    double *scores;
    int num_scores;
    double *vis_probs;
    int num_validation_points;
    olf_pose_sample_t dists;
  } scope_sample_t;


  typedef struct {
    scope_sample_t *samples;
    double *W;
    int num_samples;
    int num_samples_allocated;
  } scope_samples_t;


  typedef struct {
    int *model_ids;
    scope_sample_t *objects;
    int num_objects;
    double *scores; //dbg
    int num_scores;
  } mope_sample_t;

  typedef struct {
    mope_sample_t *samples;
    double *W;
    int num_samples;
    int num_samples_allocated;
  } mope_samples_t;
  
  pcd_t *load_pcd(char *f_pcd);                        // loads a pcd
  void pcd_free(pcd_t *pcd);                           // frees the contents of a pcd_t, but not the pointer itself
  int pcd_channel(pcd_t *pcd, char *channel_name);     // gets the index of a channel by name
  int pcd_add_channel(pcd_t *pcd, char *channel);      // adds a channel to pcd

  range_image_t *pcd_to_range_image(pcd_t *pcd, double *vp, double res, int padding);

  void load_olf_model(olf_model_t *model, char *model_file, scope_params_t *params);
  olf_model_t *load_olf_models(int *n, char *models_file, scope_params_t *params);

  void load_scope_params(scope_params_t *params, char *param_file);
  void load_mope_params(mope_params_t *params, char *param_file);
  void get_scope_model_data(scope_model_data_t *model_data, olf_model_t *model, scope_params_t *params);
  void get_scope_obs_data(scope_obs_data_t *obs_data, olf_obs_t *obs, scope_params_t *params);

  void free_scope_model_data(scope_model_data_t *data);
  void free_scope_obs_data(scope_obs_data_t *data);

struct cu_model_data_struct;
struct cu_obs_data_struct;

void test_bpa(scope_model_data_t *model_data, scope_obs_data_t *obs_data, scope_params_t *params, simple_pose_t *true_pose);

scope_samples_t *scope(scope_model_data_t *model_data, scope_obs_data_t *obs_data, scope_params_t *params, simple_pose_t *true_pose, struct cu_model_data_struct *cu_model, struct cu_obs_data_struct *cu_obs,
		       int *segment_blacklist);

mope_sample_t *mope_greedy(scope_model_data_t *models, int num_models, scope_obs_data_t *obs, scope_params_t *scope_params, mope_params_t *mope_params, 
			   struct cu_model_data_struct *cu_model, struct cu_obs_data_struct *cu_obs);
//mope_sample_t *mope_annealing(scope_model_data_t *models, int num_models, scope_obs_data_t *obs, scope_params_t *params, struct cu_model_data_struct *cu_model, struct cu_obs_data_struct *cu_obs);
mope_samples_t *annealing_with_scope(scope_model_data_t *models, int num_models, int *segment_cnts, scope_obs_data_t *obs, scope_params_t *scope_params, mope_params_t *mope_params,
				     struct cu_model_data_struct *cu_model, struct cu_obs_data_struct *cu_obs, FILE *f, int round, int *segment_blacklist);
mope_samples_t *annealing_existing_samples(scope_model_data_t *models, int num_models, int *segment_cnts, scope_obs_data_t *obs_data, int num_obs_segments, 
					   scope_params_t *scope_params, mope_params_t *mope_params, FILE *f, int round);

/*#ifdef __cplusplus
extern "C" {
#endif*/



  //
  // DEPRECATED
  //

 
  /*typedef struct {
    //double **X;
    //double **Q;
    //double *W;
    int n;

    //dbug
    //int **C_obs;
    //int **C_model;
    double **vis_probs;
    double **xyz_dists;
    double **fpfh_dists;
    double **normal_dists;
    int **range_edge_pixels;
    int **range_edge_points;
    //double **range_edge_vis_prob;
    int *num_range_edge_points;
    int **occ_edge_pixels;
    int *num_occ_edge_points;
    double **scores;
    int *num_scores;

    } olf_pose_samples_t;  */

  /* heirarchical segmentation and balls model
  typedef struct {
    int num_segments;
    int *num_balls;
    int *segment_labels;
    int *ball_labels;
    double **segment_centers;
    double *segment_radii;
    double ***ball_centers;
    double **ball_radii;
    double mean_segment_radius;
    double mean_ball_radius;
  } pcd_balls_t;
  */

  /* oriented local feature model
  typedef struct {
    pcd_t *pcd;                   // point cloud
    bingham_mix_t *bmx;           // bingham mixture models (one per cluster)
    hll_t *hll;                   // hyperspherical local likelihood models (one per cluster)
    int num_clusters;             // # local feature clusters
    double *cluster_weights;      // cluster weights
    int shape_length;             // local shape descriptor length
    double **mean_shapes;         // cluster means
    double *shape_variances;      // cluster variances

    // params
    int rot_symm;
    int num_validators;
    double lambda;
    double pose_agg_x;
    double pose_agg_q;
    double *proposal_weights;
    int cluttered;
    int num_proposal_segments;
    int *proposal_segments;
  } olf_t;
  */

  //olf_t *load_olf(char *fname);                       // loads an olf from fname.pcd and fname.bmx
  //void olf_free(olf_t *olf);                          // frees the contents of an olf_t, but not the pointer itself
  //void olf_classify_points(pcd_t *pcd, olf_t *olf);   // classify pcd points (add channel "cluster") using olf shapes

  // computes the pdf of pose (x,q) given n points from pcd w.r.t. olf (assumes points are classified)
  //double olf_pose_pdf(double *x, double *q, olf_t *olf, pcd_t *pcd, int *indices, int n);

  // samples n weighted poses (X,Q,W) using olf model "olf" and point cloud "pcd"
  //olf_pose_samples_t *olf_pose_sample(olf_t *olf, pcd_t *pcd, int n);

  // aggregate the weighted pose samples, (X,Q,W)
  //olf_pose_samples_t *olf_aggregate_pose_samples(olf_pose_samples_t *poses, olf_t *olf);

  //olf_pose_samples_t *olf_pose_samples_new(int n);          // create a new olf_pose_samples_t
  //void olf_pose_samples_free(olf_pose_samples_t *poses);    // free pose samples

  /*  
  int sample_model_point_given_model_pose(double *X, double *Q, int *c_model_prev, int n_model_prev, double *model_pmf, pcd_t *pcd_model);

  int sample_obs_correspondence_given_model_pose(double *X, double *Q, int model, pcd_t *pcd_model, int shape_length, double **obs_fxyzn, flann_index_t obs_xyzn_index, struct FLANNParameters *obs_xyzn_params, scope_params_t *params);
  void get_model_pose_distribution_from_correspondences(pcd_t *pcd_obs, pcd_t *pcd_model, int *c_obs, int n, int *c_model, double xyz_sigma, double *x, bingham_t *B);
  void sample_model_pose(pcd_t *pcd_model, int *c_model, int c, double *x0, bingham_t *B, double *x, double *q);
  void model_pose_likelihood(pcd_t *pcd_obs, pcd_t *pcd_model, int *c_obs, int *c_model, int n, double *x, double *q, bingham_t *B, double xyz_sigma, double f_sigma, double dispersion_weight, 
			     double *logp, double logp_comp[4]);
  int sample_model_correspondence_given_model_pose(pcd_t *pcd_obs, double **model_fxyzn, scope_params_t *params, struct FLANNParameters *model_xyzn_params, flann_index_t model_xyzn_index, double *x, double *q, int c2_obs, int sample_nn, int use_f);

  void get_point(double p[3], pcd_t *pcd, int idx);
  void get_normal(double p[3], pcd_t *pcd, int idx);
  void get_shape(double p[33], pcd_t *pcd, int idx);
  */



/*#ifdef __cplusplus
}
#endif */


#endif
