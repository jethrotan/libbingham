
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "bingham/util.h"
#include "bingham/olf.h"




//---------------------------- STATIC HELPER FUNCTIONS ---------------------------//


/*
 * compute quaternions given normals and principal curvatures
 */
void compute_orientation_quaternions(double ***Q, double **N, double **PCS, int num_points)
{
  int i;
  double nx, ny, nz, pcx, pcy, pcz, pcx2, pcy2, pcz2;
  double **R = new_matrix2(3,3);

  for (i = 0; i < num_points; i++) {
    nx = N[i][0];
    ny = N[i][1];
    nz = N[i][2];
    pcx = PCS[i][0];
    pcy = PCS[i][1];
    pcz = PCS[i][2];
    pcx2 = ny*pcz - nz*pcy;
    pcy2 = nz*pcx - nx*pcz;
    pcz2 = nx*pcy - ny*pcx;

    // compute "up" quaternion
    R[0][0] = nx;  R[0][1] = pcx;  R[0][2] = pcx2;
    R[1][0] = ny;  R[1][1] = pcy;  R[1][2] = pcy2;
    R[2][0] = nz;  R[2][1] = pcz;  R[2][2] = pcz2;
    rotation_matrix_to_quaternion(Q[0][i], R);

    // compute "down" quaternion
    R[0][0] = nx;  R[0][1] = -pcx;  R[0][2] = -pcx2;
    R[1][0] = ny;  R[1][1] = -pcy;  R[1][2] = -pcy2;
    R[2][0] = nz;  R[2][1] = -pcz;  R[2][2] = -pcz2;
    rotation_matrix_to_quaternion(Q[1][i], R);
  }

  free_matrix2(R);
}


/*
 * add data pointers to a pcd
 */
static void pcd_add_data_pointers(pcd_t *pcd)
{
  int i, j, num_points = pcd->num_points;
  int ch_cluster = pcd_channel(pcd, "cluster");
  int ch_x = pcd_channel(pcd, "x");
  int ch_y = pcd_channel(pcd, "y");
  int ch_z = pcd_channel(pcd, "z");
  int ch_red = pcd_channel(pcd, "red");
  int ch_green = pcd_channel(pcd, "green");
  int ch_blue = pcd_channel(pcd, "blue");
  int ch_nx = pcd_channel(pcd, "nx");
  int ch_ny = pcd_channel(pcd, "ny");
  int ch_nz = pcd_channel(pcd, "nz");
  int ch_pcx = pcd_channel(pcd, "pcx");
  int ch_pcy = pcd_channel(pcd, "pcy");
  int ch_pcz = pcd_channel(pcd, "pcz");
  int ch_pc1 = pcd_channel(pcd, "pc1");
  int ch_pc2 = pcd_channel(pcd, "pc2");
  int ch_f1 = pcd_channel(pcd, "f1");
  int ch_f33 = pcd_channel(pcd, "f33");
  //int ch_balls = pcd_channel(pcd, "balls");
  int ch_sift1 = pcd_channel(pcd, "sift1");
  int ch_sift128 = pcd_channel(pcd, "sift128");
  int ch_surfdist = pcd_channel(pcd, "surfdist");
  int ch_surfwidth = pcd_channel(pcd, "surfwidth");


  if (ch_cluster>=0) {
    //pcd->clusters = pcd->data[ch_cluster];
    safe_malloc(pcd->clusters, num_points, int);
    for (i = 0; i < num_points; i++)
      pcd->clusters[i] = (int)(pcd->data[ch_cluster][i]);
  }
  if (ch_x>=0 && ch_y>=0 && ch_z>=0) {
    pcd->points = new_matrix2(num_points, 3);
    for (i = 0; i < num_points; i++) {
      pcd->points[i][0] = pcd->data[ch_x][i];
      pcd->points[i][1] = pcd->data[ch_y][i];
      pcd->points[i][2] = pcd->data[ch_z][i];
    }
  }
  if (ch_red>=0 && ch_green>=0 && ch_blue>=0) {
    pcd->colors = new_matrix2(num_points, 3);
    pcd->lab = new_matrix2(num_points, 3);
    for (i = 0; i < num_points; i++) {
      pcd->colors[i][0] = pcd->data[ch_red][i];
      pcd->colors[i][1] = pcd->data[ch_green][i];
      pcd->colors[i][2] = pcd->data[ch_blue][i];
      rgb2lab(pcd->lab[i], pcd->colors[i]);
    }
  }
  if (ch_nx>=0 && ch_ny>=0 && ch_nz>=0) {
    pcd->normals = new_matrix2(num_points, 3);
    for (i = 0; i < num_points; i++) {
      pcd->normals[i][0] = pcd->data[ch_nx][i];
      pcd->normals[i][1] = pcd->data[ch_ny][i];
      pcd->normals[i][2] = pcd->data[ch_nz][i];
    }
  }
  if (ch_pcx>=0 && ch_pcy>=0 && ch_pcz>=0) {
    pcd->principal_curvatures = new_matrix2(num_points, 3);
    for (i = 0; i < num_points; i++) {
      pcd->principal_curvatures[i][0] = pcd->data[ch_pcx][i];
      pcd->principal_curvatures[i][1] = pcd->data[ch_pcy][i];
      pcd->principal_curvatures[i][2] = pcd->data[ch_pcz][i];
    }
  }
  if (ch_pc1>=0 && ch_pc2>=0) {
    pcd->pc1 = pcd->data[ch_pc1];
    pcd->pc2 = pcd->data[ch_pc2];
  }
  if (ch_f1>=0 && ch_f33>=0) {
    pcd->shape_length = 33;
    pcd->shapes = new_matrix2(num_points, pcd->shape_length);
    for (i = 0; i < num_points; i++)
      for (j = 0; j < pcd->shape_length; j++)
	pcd->shapes[i][j] = pcd->data[ch_f1 + j][i];
  }
  if (ch_sift1>=0 && ch_sift128>=0) {
    pcd->sift_length = 128;
    pcd->sift = new_matrix2(num_points, pcd->sift_length);
    for (i = 0; i < num_points; i++)
      for (j = 0; j < pcd->sift_length; j++)
	pcd->sift[i][j] = pcd->data[ch_sift1 + j][i];
  }
  if (ch_surfdist>=0 && ch_surfwidth>=0) {
    pcd->sdw_length = 2;
    pcd->sdw = new_matrix2(num_points, pcd->sdw_length);
    for (i = 0; i < num_points; i++) {
      pcd->sdw[i][0] = pcd->data[ch_surfdist][i];
      pcd->sdw[i][1] = pcd->data[ch_surfwidth][i];
    }
  }

  // add quaternion orientation features
  if (ch_nx>=0 && ch_ny>=0 && ch_nz>=0 && ch_pcx>=0 && ch_pcy>=0 && ch_pcz>=0) {
    pcd->quaternions[0] = new_matrix2(pcd->num_points, 4);
    pcd->quaternions[1] = new_matrix2(pcd->num_points, 4);
    compute_orientation_quaternions(pcd->quaternions, pcd->normals, pcd->principal_curvatures, pcd->num_points);
  }

  // add points kdtree
  //double **X = new_matrix2(pcd->num_points, 3);
  //transpose(X, pcd->points, 3, pcd->num_points);
  //pcd->points_kdtree = kdtree(X, pcd->num_points, 3);
  //free_matrix2(X);

  // add balls model
  //if (ch_balls>=0)
  //  pcd_add_balls(pcd);
}


/*
 * free data pointers in a pcd
 */
static void pcd_free_data_pointers(pcd_t *pcd)
{
  if (pcd->points)
    free_matrix2(pcd->points);
  if (pcd->colors)
    free_matrix2(pcd->colors);
  if (pcd->lab)
    free_matrix2(pcd->lab);
  if (pcd->normals)
    free_matrix2(pcd->normals);
  if (pcd->principal_curvatures)
    free_matrix2(pcd->principal_curvatures);
  if (pcd->shapes)
    free_matrix2(pcd->shapes);
  if (pcd->sift)
    free_matrix2(pcd->sift);
  if (pcd->sdw)
    free_matrix2(pcd->sdw);
  if (pcd->clusters)
    free(pcd->clusters);

  if (pcd->quaternions[0])
    free_matrix2(pcd->quaternions[0]);
  if (pcd->quaternions[1])
    free_matrix2(pcd->quaternions[1]);

  //if (pcd->points_kdtree)
  //  kdtree_free(pcd->points_kdtree);
  //if (pcd->balls)
  //  pcd_free_balls(pcd);
}





//---------------------------- EXTERNAL API ---------------------------//



/*
 * loads a pcd
 */
pcd_t *load_pcd(char *f_pcd)
{
  int i, j;
  FILE *f = fopen(f_pcd, "r");

  if (f == NULL) {
    fprintf(stderr, "Invalid filename: %s", f_pcd);
    return NULL;
  }

  pcd_t *pcd;
  safe_calloc(pcd, 1, pcd_t);

  char sbuf[10000], *s = sbuf;
  while (!feof(f)) {
    s = sbuf;
    if (fgets(s, 10000, f)) {

      if (!wordcmp(s, "COLUMNS", " \t\n") || !wordcmp(s, "FIELDS", " \t\n")) {
	s = sword(s, " \t", 1);
	pcd->channels = split(s, " \t\n", &pcd->num_channels);


	/* TODO: make a file converter?
	   replace_word(pcd->channels, pcd->num_channels, "normal_x", "nx");
	   replace_word(pcd->channels, pcd->num_channels, "normal_y", "ny");
	   replace_word(pcd->channels, pcd->num_channels, "normal_z", "nz");
	   replace_word(pcd->channels, pcd->num_channels, "principal_curvature_x", "pcx");
	   replace_word(pcd->channels, pcd->num_channels, "principal_curvature_y", "pcy");
	   replace_word(pcd->channels, pcd->num_channels, "principal_curvature_z", "pcz");
	   //s = strrep(s, 'fpfh', ['f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 ' ...
	   //                       'f15 f16 f17 f18 f19 f20 f21 f22 f23 f24 f25 f26 f27 f28 f29 f30 f31 f32 f33']);
	   */

      }
      else if (!wordcmp(s, "POINTS", " \t\n")) {
	s = sword(s, " \t", 1);
	sscanf(s, "%d", &pcd->num_points);
      }
      else if (!wordcmp(s, "DATA", " \t\n")) {
	s = sword(s, " \t", 1);
	if (wordcmp(s, "ascii", " \t\n")) {  // NOTE(sanja): should this be !wordcmp?
	  fprintf(stderr, "Error: only ascii pcd files are supported.\n");
	  pcd_free(pcd);
	  free(pcd);
	  return NULL;
	}

	safe_calloc(pcd->data, pcd->num_channels, double *);
	for (i = 0; i < pcd->num_channels; i++)
	  safe_calloc(pcd->data[i], pcd->num_points, double);

	for (i = 0; i < pcd->num_points; i++) {
	  s = sbuf;
	  if (fgets(s, 10000, f) == NULL)
	    break;
	  for (j = 0; j < pcd->num_channels; j++) {
	    if (sscanf(s, "%lf", &pcd->data[j][i]) < 1)
	      break;
	    s = sword(s, " \t", 1);
	  }
	  if (j < pcd->num_channels)
	    break;
	}

	if (i < pcd->num_points) {
	  fprintf(stderr, "Error: corrupt pcd data at row %d\n", i);
	  pcd_free(pcd);
	  free(pcd);
	  return NULL;
	}
      }
    }
  }

  pcd_add_data_pointers(pcd);

  return pcd;
}


/*
 * frees the contents of a pcd_t, but not the pointer itself
 */
void pcd_free(pcd_t *pcd)
{
  int i;

  if (pcd == NULL)
    return;

  if (pcd->channels) {
    for (i = 0; i < pcd->num_channels; i++)
      if (pcd->channels[i])
	free(pcd->channels[i]);
    free(pcd->channels);
  }

  if (pcd->data) {
    for (i = 0; i < pcd->num_channels; i++)
      free(pcd->data[i]);
    free(pcd->data);
  }

  pcd_free_data_pointers(pcd);
}


/*
 * gets the index of a channel by name
 */
int pcd_channel(pcd_t *pcd, char *channel_name)
{
  int i;
  for (i = 0; i < pcd->num_channels; i++)
    if (!strcmp(pcd->channels[i], channel_name))
      return i;

  return -1;
}


/*
 * adds a channel to pcd
 */
int pcd_add_channel(pcd_t *pcd, char *channel)
{
  int ch = pcd_channel(pcd, channel);
  if (ch >= 0) {
    printf("Warning: channel %s already exists\n", channel);
    return ch;
  }

  // add channel name
  ch = pcd->num_channels;
  pcd->num_channels++;
  safe_realloc(pcd->channels, pcd->num_channels, char *);
  safe_calloc(pcd->channels[ch], strlen(channel) + 1, char);
  strcpy(pcd->channels[ch], channel);
  
  // add space for data
  safe_realloc(pcd->data, pcd->num_channels, double *);
  safe_calloc(pcd->data[ch], pcd->num_points, double);

  return ch;
}






//------------------------------- Bingham Procrustean Alignment -------------------------------//


range_image_t *pcd_to_range_image_from_template(pcd_t *pcd, range_image_t *R0)
{
  double *vp = R0->vp;
  double **Q;
  Q = new_matrix2(3,3);
  quaternion_to_rotation_matrix(Q, &vp[3]);

  int i, j, n = pcd->num_points;
  double X[n], Y[n], D[n];
  for (i = 0; i < n; i++) {
    // get point (w.r.t. viewpoint)
    double p[3];
    sub(p, pcd->points[i], vp, 3);
    matrix_vec_mult(p, Q, p, 3, 3);
    // compute range image coordinates
    D[i] = norm(p,3);
    X[i] = atan2(p[0], p[2]);
    Y[i] = asin(p[1]/D[i]);
  }

  range_image_t *R;
  safe_calloc(R, 1, range_image_t);
  *R = *R0;  // shallow copy

  int w = R->w, h = R->h;
  R->image = new_matrix2(w, h);
  R->idx = new_matrix2i(w, h);

  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      R->image[i][j] = -1.0;
      R->idx[i][j] = -1;
    }
  }

  // fill in range image
  for (i = 0; i < n; i++) {
    int cx = (int)floor( (X[i] - R->min[0]) / R->res);
    int cy = (int)floor( (Y[i] - R->min[1]) / R->res);
    double d = D[i];
    double r = R->image[cx][cy];
    if (r < 0 || r > d) {
      R->image[cx][cy] = d;
      R->idx[cx][cy] = i;
    }
  }

  free_matrix2(Q);

  return R;
}


range_image_t *pcd_to_range_image(pcd_t *pcd, double *vp, double res)
{
  int i, j, n = pcd->num_points;
  double **Q;
  if (vp != NULL) {
    Q = new_matrix2(3,3);
    quaternion_to_rotation_matrix(Q, &vp[3]);
  }

  double X[n], Y[n], D[n];
  for (i = 0; i < n; i++) {
    // get point (w.r.t. viewpoint)
    double p[3];
    for (j = 0; j < 3; j++)
      p[j] = pcd->points[i][j];
    if (vp != NULL) {
      for (j = 0; j < 3; j++)
	p[j] -= vp[j];
      double p2[3];
      matrix_vec_mult(p2, Q, p, 3, 3);
      for (j = 0; j < 3; j++)
	p[j] = p2[j];
    }
    // compute range image coordinates
    D[i] = norm(p,3);
    X[i] = atan2(p[0], p[2]);
    Y[i] = asin(p[1]/D[i]);
  }

  // create range image
  range_image_t *R;
  safe_calloc(R, 1, range_image_t);
  if (vp != NULL)
    for (i = 0; i < 7; i++)
      R->vp[i] = vp[i];
  else
    R->vp[3] = 1.0;  // identity transform: (0,0,0,1,0,0,0)
  R->res = res;

  R->min[0] = min(X,n) - res/2.0;
  R->min[1] = min(Y,n) - res/2.0;
  int w0 = (int)ceil(2*M_PI/res);
  double **image = new_matrix2(w0,w0);
  int **idx = new_matrix2i(w0,w0);

  for (i = 0; i < w0; i++) {
    for (j = 0; j < w0; j++) {
      image[i][j] = -1.0;
      idx[i][j] = -1;
    }
  }

  // fill in range image
  for (i = 0; i < n; i++) {
    int cx = (int)floor( (X[i] - R->min[0]) / res);
    int cy = (int)floor( (Y[i] - R->min[1]) / res);
    double d = D[i];

    double r = image[cx][cy];
    if (r < 0 || r > d) {
      image[cx][cy] = d;
      idx[cx][cy] = i;
      R->w = MAX(R->w, cx+1);
      R->h = MAX(R->h, cy+1);
    }
  }

  // crop empty range pixels
  R->image = new_matrix2(R->w, R->h);
  R->idx = new_matrix2i(R->w, R->h);
  for (i = 0; i < R->w; i++) {
    for (j = 0; j < R->h; j++) {
      R->image[i][j] = image[i][j];
      R->idx[i][j] = idx[i][j];
    }
  }

  // cleanup
  free_matrix2(image);
  free_matrix2i(idx);
  if (vp != NULL)
    free_matrix2(Q);

  return R;
}


int range_image_xyz2sub(int *i, int *j, range_image_t *range_image, double *xyz)
{
  //TODO: use range image viewpoint

  double d = norm(xyz, 3);
  double x = atan2(xyz[0], xyz[2]);
  double y = asin(xyz[1] / d);

  int cx = (int)floor((x - range_image->min[0]) / range_image->res);
  int cy = (int)floor((y - range_image->min[1]) / range_image->res);

  *i = cx;
  *j = cy;

  return cx>=0 && cy>=0 && (cx < range_image->w) && (cy < range_image->h);
}


void range_image_find_nn(int *nn_idx, double *nn_d2, double **query_xyz, double **query, int query_num, int query_len,
			 double **data, range_image_t *range_image, int search_radius_pixels)
{
  int i;
  for (i = 0; i < query_num; i++) {
    int imin = 0;
    double d2min = dist2(query[i], data[0], query_len);
    int x, y;
    int inbounds = range_image_xyz2sub(&x, &y, range_image, query_xyz[i]);
    if (inbounds) {
      int r = search_radius_pixels;
      int x0 = MAX(0, x-r);
      int x1 = MIN(x+r, range_image->w - 1);
      int y0 = MAX(0, y-r);
      int y1 = MIN(y+r, range_image->h - 1);
      for (x = x0; x <= x1; x++) {
	for (y = y0; y <= y1; y++) {
	  int idx = range_image->idx[x][y];
	  if (idx < 0)
	    continue;
	  double d2 = dist2(query[i], data[idx], query_len);
	  if (imin < 0 || d2 < d2min) {
	    imin = idx;
	    d2min = d2;
	  }
	}
      }
    }
    nn_idx[i] = imin;
    nn_d2[i] = d2min;
  }
}


double *compute_model_saliency(pcd_t *pcd_model)
{
  double epsilon = 1e-50;

  int i;
  double *model_pmf;
  int n = pcd_model->num_points;
  safe_calloc(model_pmf, n, double);
  for (i = 0; i < pcd_model->num_points; i++) {
    double pc1 = MAX(pcd_model->pc1[i], epsilon);
    double pc2 = MAX(pcd_model->pc2[i], epsilon);
    model_pmf[i] = pc1/pc2 - 1.0;
    model_pmf[i] = MIN(model_pmf[i], 10);
  }
  mult(model_pmf, model_pmf, 1.0/sum(model_pmf, n), n);

  return model_pmf;
}


int find_obs_sift_matches(int *obs_idx, int *model_idx, pcd_t *sift_obs, pcd_t *sift_model, double sift_dthresh)
{
  const int sift_len = 128;

  int n1 = sift_obs->num_points;
  int n2 = sift_model->num_points;

  double **desc1 = sift_obs->sift;  //new_matrix2(n1, sift_len);
  double **desc2 = sift_model->sift;  //new_matrix2(n2, sift_len);
  //transpose(desc1, sift_obs->sift, sift_len, n1);
  //transpose(desc2, sift_model->sift, sift_len, n2);

  int num_matches = 0;
  int i,j;
  for (i = 0; i < n1; i++) {
    double dmin = 100000.0;
    int jmin = 0;
    for (j = 0; j < n2; j++) {
      double d = acos(dot(desc1[i], desc2[j], sift_len));
      if (d < dmin) {
	dmin = d;
	jmin = j;
      }
    }
    if (dmin < sift_dthresh) {
      obs_idx[num_matches] = i;
      model_idx[num_matches] = jmin;
      num_matches++;
    }
  }

  //free_matrix2(desc1);
  //free_matrix2(desc2);

  return num_matches;
}


// assumes B is already allocated
void olf_to_bingham(bingham_t *B, int idx, pcd_t *pcd)
{
  double epsilon = 1e-50;

  double r1[3] = {pcd->normals[idx][0], pcd->normals[idx][1], pcd->normals[idx][2]};
  double r2[3] = {pcd->principal_curvatures[idx][0], pcd->principal_curvatures[idx][1], pcd->principal_curvatures[idx][2]};
  double r3[3];
  cross(r3, r1, r2);
  double *R[3] = {r1,r2,r3};

  // v1: mode
  double v1[4];
  rotation_matrix_to_quaternion(v1, R);
  quaternion_inverse(v1, v1);

  // B->V[2]: rotation about the normal vector
  int i;
  for (i = 0; i < 3; i++) {
    r2[i] = -r2[i];
    r3[i] = -r3[i];
  }
  rotation_matrix_to_quaternion(B->V[2], R);
  quaternion_inverse(B->V[2], B->V[2]);

  double **V1 = new_matrix2(4,4);  // v1's projection matrix 
  outer_prod(V1, v1, v1, 4, 4);
  mult(V1[0], V1[0], -1, 16);
  for (i = 0; i < 4; i++)
    V1[i][i] += 1.0;
  double **V2 = new_matrix2(4,4);  // B->V[2]'s projection matrix
  outer_prod(V2, B->V[2], B->V[2], 4, 4);
  mult(V2[0], V2[0], -1, 16);
  for (i = 0; i < 4; i++)
    V2[i][i] += 1.0;
  double **V12 = new_matrix2(4,4);
  matrix_mult(V12, V1, V2, 4, 4, 4);

  // find a third orthogonal vector, B->V[1]
  double v3[4];
  for (i = 0; i < 4; i++)
    v3[i] = frand() - 0.5;
  matrix_vec_mult(B->V[1], V12, v3, 4, 4);
  normalize(B->V[1], B->V[1], 4);

  double **V3 = V1;  // B->V[1]'s projection matrix
  outer_prod(V3, B->V[1], B->V[1], 4, 4);
  mult(V3[0], V3[0], -1, 16);
  for (i = 0; i < 4; i++)
    V3[i][i] += 1.0;
  double **V123 = V2;
  matrix_mult(V123, V12, V3, 4, 4, 4);

  // find a fourth orthogonal vector, B->V[0]
  double v4[4];
  for (i = 0; i < 4; i++)
    v4[i] = frand() - 0.5;
  matrix_vec_mult(B->V[0], V123, v4, 4, 4);
  normalize(B->V[0], B->V[0], 4);

  // compute concentration params
  double pc1 = MAX(pcd->pc1[idx], epsilon);
  double pc2 = MAX(pcd->pc2[idx], epsilon);
  double z3 = MIN(10*(pc1/pc2 - 1), 100);
  B->Z[0] = -100;
  B->Z[1] = -100;
  B->Z[2] = -z3;

  // look up the normalization constant
  bingham_F(B);

  free_matrix2(V1);
  free_matrix2(V2);
  free_matrix2(V12);
}


void model_pose_from_one_correspondence(double *x, double *q, int c_obs, int c_model, pcd_t *pcd_obs, pcd_t *pcd_model)
{
  // compute rotation
  bingham_t B;
  bingham_alloc(&B, 4);
  olf_to_bingham(&B, c_model, pcd_model);
  int flip = (frand() < .5);
  double *q_feature_to_world = pcd_obs->quaternions[flip][c_obs];
  double q_feature_to_model[4], q_model_to_feature[4];
  double *q_feature_to_model_ptr[1] = {q_feature_to_model};
  bingham_sample(q_feature_to_model_ptr, &B, 1);
  quaternion_inverse(q_model_to_feature, q_feature_to_model);
  quaternion_mult(q, q_feature_to_world, q_model_to_feature);
  double **R = new_matrix2(3,3);
  quaternion_to_rotation_matrix(R, q);

  // compute translation
  double *obs_point = pcd_obs->points[c_obs];
  double *model_point = pcd_model->points[c_model];
  double model_point_rot[3];
  matrix_vec_mult(model_point_rot, R, model_point, 3, 3);
  sub(x, obs_point, model_point_rot, 3);

  // cleanup
  free_matrix2(R);
}


void get_validation_points(int *idx, pcd_t *pcd_model, int num_validation_points)
{
  int i;
  if (num_validation_points == pcd_model->num_points)  // use all the points
    for (i = 0; i < pcd_model->num_points; i++)
      idx[i] = i;
  else
    randperm(idx, pcd_model->num_points, num_validation_points);
}

double compute_visibility_prob(double *point, double *normal, range_image_t *obs_range_image, double range_sigma)
{
  if (dot(point, normal, 3) >= 0)  // normals pointing away
    return 0.0;

  int x, y;
  int inbounds = range_image_xyz2sub(&x, &y, obs_range_image, point);
  if (!inbounds)
    return 0.0;

  double model_range = norm(point, 3);
  double obs_range = obs_range_image->image[x][y];

  /* let obs_range be the max of its neighbors
  if (x > 0)
    obs_range = MAX(obs_range, obs_range_image->image[x-1][y]);
  if (y > 0)
    obs_range = MAX(obs_range, obs_range_image->image[x][y-1]);
  if (x < obs_range_image->w - 1)
    obs_range = MAX(obs_range, obs_range_image->image[x+1][y]);
  if (y < obs_range_image->h - 1)
    obs_range = MAX(obs_range, obs_range_image->image[x][y+1]);
  */

  double dR = model_range - obs_range;
  return (dR < 0 ? 1.0 : normpdf(dR/range_sigma, 0, 1) / .3989);  // .3989 = normpdf(0,0,1)
}

double **get_sub_cloud_at_pose(pcd_t *pcd, int *idx, int n, double *x, double *q)
{
  double **cloud = new_matrix2(n,3);
  double **R = new_matrix2(3,3);
  quaternion_to_rotation_matrix(R,q);
  int i;
  for (i = 0; i < n; i++) {
    memcpy(cloud[i], pcd->points[idx[i]], 3*sizeof(double));
    matrix_vec_mult(cloud[i], R, cloud[i], 3, 3);
    add(cloud[i], cloud[i], x, 3);
  }
  free_matrix2(R);

  return cloud;
}

double **get_sub_cloud_normals_rotated(pcd_t *pcd, int *idx, int n, double *q)
{
  double **normals = new_matrix2(n,3);
  double **R = new_matrix2(3,3);
  quaternion_to_rotation_matrix(R,q);
  int i;
  for (i = 0; i < n; i++) {
    memcpy(normals[i], pcd->normals[idx[i]], 3*sizeof(double));
    matrix_vec_mult(normals[i], R, normals[i], 3, 3);
  }
  free_matrix2(R);

  return normals;
}

double **get_sub_cloud_fpfh(pcd_t *pcd, int *idx, int n)
{
  double **F = new_matrix2(n, pcd->shape_length);
  reorder_rows(F, pcd->shapes, idx, n, pcd->shape_length);
  return F;
}

double **get_sub_cloud_sdw(pcd_t *pcd, int *idx, int n, scope_params_t *params)
{
  double surfdist_thresh = params->surfdist_thresh;
  double surfwidth_thresh = params->surfwidth_thresh;

  double **sdw = new_matrix2(n, 2);
  reorder_rows(sdw, pcd->sdw, idx, n, 2);

  int i;
  for (i = 0; i < n; i++) {
    sdw[i][0] = MIN(sdw[i][0], surfdist_thresh);
    sdw[i][1] = MIN(sdw[i][1], surfwidth_thresh);
  }

  return sdw;
}

double **get_sub_cloud_lab(pcd_t *pcd, int *idx, int n)
{
  double **lab = new_matrix2(n,3);
  reorder_rows(lab, pcd->lab, idx, n, 3);
  return lab;
}

double **get_xyzn_features(double **points, double **normals, int n, scope_params_t *params)
{
  int i, j;
  double xyz_weight = params->xyz_weight;
  double normal_weight = params->normal_weight;
  double **xyzn = new_matrix2(n,6);
  for (i = 0; i <n; i++) {
    for (j = 0; j < 3; j++) {
      xyzn[i][j] = xyz_weight * points[i][j];
      xyzn[i][j+3] = normal_weight * normals[i][j];
    }
  }

  return xyzn;
}

double **get_fxyzn_features(double **fpfh, double **points, double **normals, int n, int fpfh_length, scope_params_t *params)
{
  int i, j;
  double xyz_weight = params->xyz_weight;
  double normal_weight = params->normal_weight;
  double **fxyzn = new_matrix2(n, fpfh_length + 6);
  for (i = 0; i <n; i++) {
    memcpy(fxyzn[i], fpfh[i], fpfh_length*sizeof(double));
    for (j = 0; j < 3; j++) {
      fxyzn[i][j+fpfh_length] = xyz_weight * points[i][j];
      fxyzn[i][j+3+fpfh_length] = normal_weight * normals[i][j];
    }
  }

  return fxyzn;
}

float **get_fsurf_features(double **fpfh, double **sdw, int n, int fpfh_length, scope_params_t *params)
{
  int i, j;
  double surfdist_thresh = params->surfdist_thresh;
  double surfwidth_thresh = params->surfwidth_thresh;
  double surfdist_weight = params->surfdist_weight;
  double surfwidth_weight = params->surfwidth_weight;

  float **fsurf = new_matrix2f(n, fpfh_length + 2);
  for (i = 0; i < n; i++) {
    for (j = 0; j < fpfh_length; j++)
      fsurf[i][j] = fpfh[i][j];
    double surfdist, surfwidth;
    if (sdw) {
      surfdist = MIN(sdw[i][0], surfdist_thresh);
      surfwidth = MIN(sdw[i][1], surfwidth_thresh);
    } else {
      surfdist = surfdist_thresh;
      surfwidth = surfwidth_thresh;
    }
    fsurf[i][fpfh_length] = surfdist_weight * surfdist;
    fsurf[i][fpfh_length+1] = surfwidth_weight * surfwidth;
  }

  return fsurf;
}

double compute_xyzn_score(double *nn_d2, double *vis_prob, int n, scope_params_t *params)
{
  int i;
  double score = 0.0;
  double range_sigma = params->range_sigma;
  double xyz_weight = params->xyz_weight;
  for (i = 0; i < n; i++) {
    double d = sqrt(nn_d2[i]) / xyz_weight;  // TODO: make this a param
    d = MIN(d, 3*range_sigma);               // TODO: make this a param
    score += vis_prob[i] * log(normpdf(d, 0, range_sigma));
  }

  score -= log(normpdf(0,0,range_sigma));
  return score;
}

double compute_fpfh_score(double **fpfh, int *nn_idx, double *vis_prob, int n, pcd_t *pcd_obs, scope_params_t *params)
{
  double score = 0.0;
  double f_sigma = params->f_sigma;
  int i;
  for (i = 0; i < n; i++) {
    double *obs_f = pcd_obs->shapes[nn_idx[i]];
    double df = dist(fpfh[i], obs_f, pcd_obs->shape_length);
    df = MIN(df, 4*f_sigma);  // TODO: make this a param
    score += vis_prob[i] * log(normpdf(df, 0, 2*f_sigma));
  }

  score -= log(normpdf(0,0,2*f_sigma));
  return params->f_weight * score;
}

double compute_sdw_score(double **sdw, int *nn_idx, double *vis_prob, int n, pcd_t *pcd_obs, scope_params_t *params)
{
  double surfdist_thresh = params->surfdist_thresh;
  double surfwidth_thresh = params->surfwidth_thresh;
  double surfdist_sigma = params->surfdist_sigma;
  double surfwidth_sigma = params->surfwidth_sigma;

  double score = 0.0;
  int i;
  for (i = 0; i < n; i++) {
    double *obs_sdw = pcd_obs->sdw[nn_idx[i]];
    double obs_surfdist = MIN(obs_sdw[0], surfdist_thresh);
    double obs_surfwidth = MIN(obs_sdw[1], surfwidth_thresh);
    score += vis_prob[i] * log(normpdf(sdw[i][0] - obs_surfdist, 0, surfdist_sigma));
    score += vis_prob[i] * log(normpdf(sdw[i][1] - obs_surfwidth, 0, surfwidth_sigma));
  }

  score -= log(normpdf(0,0,surfdist_sigma));
  score -= log(normpdf(0,0,surfwidth_sigma));

  return score;
}

double compute_lab_score(double **lab, int *nn_idx, double *vis_prob, int n, pcd_t *pcd_obs, scope_params_t *params)
{
  double score = 0.0;
  int i;
  double dlab[3];
  double L_weight = params->L_weight;
  double lab_sigma = params->lab_sigma;
  for (i = 0; i < n; i++) {
    double *obs_lab = pcd_obs->lab[nn_idx[i]];
    dlab[0] = L_weight * (lab[i][0] - obs_lab[0]);
    dlab[1] = lab[i][1] - obs_lab[1];
    dlab[2] = lab[i][2] - obs_lab[2];
    double d = norm(dlab, 3);
    d = MIN(d, 2*lab_sigma);  // TODO: make this a param
    score += vis_prob[i] * log(normpdf(d, 0, lab_sigma));
  }

  score -= log(normpdf(0, 0, lab_sigma));
  return score;
}

double compute_vis_score(double *vis_prob, int n, scope_params_t *params)
{
  return params->vis_weight * log(sum(vis_prob, n) / (double) n);
}

//dbug: useful debugging data from model_placement_score
static int mps_idx_[100000];
static double mps_vis_prob_[100000];

double model_placement_score(double *x, double *q, pcd_t *pcd_model, pcd_t *pcd_obs, range_image_t *obs_range_image, double **obs_xyzn, scope_params_t *params)
{
  // get model validation points
  int num_validation_points = (params->num_validation_points > 0 ? params->num_validation_points : pcd_model->num_points);
  int idx[num_validation_points];
  get_validation_points(idx, pcd_model, num_validation_points);

  if (params->verbose)
    memcpy(mps_idx_, idx, num_validation_points*sizeof(int));

  // extract transformed model validation features
  double **cloud = get_sub_cloud_at_pose(pcd_model, idx, num_validation_points, x, q);
  double **cloud_normals = get_sub_cloud_normals_rotated(pcd_model, idx, num_validation_points, q);
  //double **cloud_f = get_sub_cloud_fpfh(pcd_model, idx, num_validation_points);
  double **cloud_sdw = get_sub_cloud_sdw(pcd_model, idx, num_validation_points, params);
  double **cloud_lab = get_sub_cloud_lab(pcd_model, idx, num_validation_points);
  double **cloud_xyzn = get_xyzn_features(cloud, cloud_normals, num_validation_points, params);

  // compute p(visibile)
  double vis_prob[num_validation_points];
  int i;
  for (i = 0; i < num_validation_points; i++)
    vis_prob[i] = compute_visibility_prob(cloud[i], cloud_normals[i], obs_range_image, params->range_sigma);
  double vis_pmf[num_validation_points];
  normalize_pmf(vis_pmf, vis_prob, num_validation_points);

  if (params->verbose)
    memcpy(mps_vis_prob_, vis_prob, num_validation_points*sizeof(double));

  // compute nearest neighbors
  int nn_idx[num_validation_points];  memset(nn_idx, 0, num_validation_points*sizeof(int));
  double nn_d2[num_validation_points];  memset(nn_d2, 0, num_validation_points*sizeof(double));
  int search_radius = 5;  // pixels
  for (i = 0; i < num_validation_points; i++)
    if (vis_prob[i] > .01)
      range_image_find_nn(&nn_idx[i], &nn_d2[i], &cloud[i], &cloud_xyzn[i], 1, 6, obs_xyzn, obs_range_image, search_radius);
      //range_image_find_nn(&nn_idx[i], &nn_d2[i], &cloud[i], &cloud[i], 1, 3, pcd_obs->points, obs_range_image, search_radius);

  double xyzn_score = compute_xyzn_score(nn_d2, vis_pmf, num_validation_points, params);
  //double f_score = compute_fpfh_score(cloud_f, nn_idx, vis_pmf, num_validation_points, pcd_obs, params);
  double sdw_score = compute_sdw_score(cloud_sdw, nn_idx, vis_pmf, num_validation_points, pcd_obs, params);
  double lab_score = compute_lab_score(cloud_lab, nn_idx, vis_pmf, num_validation_points, pcd_obs, params);
  double vis_score = compute_vis_score(vis_prob, num_validation_points, params);

  double score = xyzn_score + sdw_score + lab_score + vis_score;

  if (params->verbose) {
    printf("score_comp = (%f, %f, %f, %f)\n", xyzn_score, sdw_score, lab_score, vis_score);
    printf("score = %f\n", score);
  }

  // cleanup
  free_matrix2(cloud);
  free_matrix2(cloud_normals);
  //free_matrix2(cloud_f);
  free_matrix2(cloud_sdw);
  free_matrix2(cloud_lab);
  free_matrix2(cloud_xyzn);

  return score;
}

/*
 * Run model_placement_score() multiple times and average the results.
 */
double model_placement_score_multi(double *x, double *q, pcd_t *pcd_model, pcd_t *pcd_obs, range_image_t *obs_range_image,
				   double **obs_xyzn, scope_params_t *params, int num_trials)
{
  int i;
  double score = 0.0;
  for (i = 0; i < num_trials; i++)
    score += model_placement_score(x, q, pcd_model, pcd_obs, obs_range_image, obs_xyzn, params);
  return score / (double) num_trials;
}

void get_good_poses(simple_pose_t *true_pose, int n, double **X, double **Q, int good_pose_idx[], int great_pose_idx[],
		    int *num_good_poses, int *num_great_poses)
{
  *num_good_poses = 0;
  *num_great_poses = 0;
  int i;
  double dx[3];
  for (i = 0; i < n; ++i) {
    sub(dx, X[i], true_pose->X, 3);
    double dq = acos(fabs(dot(Q[i], true_pose->Q, 4)));
    int good_pose = (norm(dx,3) < 0.05) && (dq < M_PI/8.0);
    int great_pose = (norm(dx,3) < 0.025) && (dq < M_PI/16.0);
    if (good_pose)
      good_pose_idx[(*num_good_poses)++] = i;
    if (great_pose)
      great_pose_idx[(*num_great_poses)++] = i;
  }
}

void print_good_poses(simple_pose_t *true_pose, double **X, double **Q, int n)
{
  int num_good_poses = 0;
  int num_great_poses = 0;
  int good_pose_idx[n], great_pose_idx[n];
  get_good_poses(true_pose, n, X, Q, good_pose_idx, great_pose_idx, &num_good_poses, &num_great_poses);
  printf("Found %d/%d good poses, %d/%d great poses\n", num_good_poses, n, num_great_poses, n);
}

void print_good_poses_verbose(simple_pose_t *true_pose, double **X, double **Q, double *W, double w_sigma, int n)
{
  int num_good_poses = 0;
  int num_great_poses = 0;
  int good_pose_idx[n], great_pose_idx[n];
  get_good_poses(true_pose, n, X, Q, good_pose_idx, great_pose_idx, &num_good_poses, &num_great_poses);
  printf("Found %d/%d good poses, %d/%d great poses\n", num_good_poses, n, num_great_poses, n);

  int i;
  for (i = 0; i < n; i++) {
    if (ismemberi(i, great_pose_idx, num_great_poses))
      printf("*** W[%d] = %.2f +- %.2f\n", i, W[i], w_sigma);
    else if (ismemberi(i, good_pose_idx, num_good_poses))
      printf("* W[%d] = %.2f +- %.2f\n", i, W[i], w_sigma);
    else
      printf("W[%d] = %.2f +- %.2f\n", i, W[i], w_sigma);
  }
}

inline void get_point(double p[3], pcd_t *pcd, int idx)
{
  memcpy(p, pcd->points[idx], 3*sizeof(double));
}

inline void get_normal(double n[3], pcd_t *pcd, int idx)
{
  memcpy(n, pcd->normals[idx], 3*sizeof(double));
}

void orthogonal_vector(double *w, double *v, int n)
{
  int i;
  double w_proj[n];
  for (i = 0; i < n; i++)
    w[i] = normrand(0,1);
  proj(w_proj, w, v, n);
  sub(w, w, w_proj, n);
  normalize(w, w, n);
}

void vector_to_possible_quaternion(double *q, double *v)
{
  if (find_first_non_zero(v, 3) == -1) {
    q[0] = 1;
    q[1] = q[2] = q[3] = 0;
    return;
  }

  double **r = new_matrix2(3, 3);
  normalize(r[0], v, 3);
  orthogonal_vector(r[1], r[0], 3);
  cross(r[2], r[0], r[1]);
  transpose(r, r, 3, 3);
  
  rotation_matrix_to_quaternion(q, r);
  free_matrix2(r);
}

int sample_model_point_given_model_pose(double *x, double *q, int *c_model_prev, int n_model_prev, double* model_cmf, pcd_t *pcd_model)
{
  double **R = new_matrix2(3,3);
  quaternion_to_rotation_matrix(R,q);

  int i, c_model, found_point = 0;
  for (i = 0; i < 20; i++) {

    if (i < 10)  // try sampling from model_cmf
      c_model = cmfrand(model_cmf, pcd_model->num_points);
    else  // if that doesn't work, try uniform sampling
      c_model = irand(pcd_model->num_points);

    if (!ismemberi(c_model, c_model_prev, n_model_prev)) {  // if a point is new
      double n[3], xyz[3];
      matrix_vec_mult(n, R, pcd_model->normals[c_model], 3, 3);
      matrix_vec_mult(xyz, R, pcd_model->points[c_model], 3, 3);
      add(xyz, xyz, x, 3);

      if (dot(n, xyz, 3) < 0) {  // check if normals are pointing towards camera
	found_point = 1;
	break;
      }
    }
  }

  free_matrix2(R);

  return (found_point ? c_model : -1);
}

int sample_obs_correspondence_given_model_pose(double *x, double *q, int c_model, pcd_t *pcd_model, double **obs_fxyzn,
					       flann_index_t obs_xyzn_index, struct FLANNParameters *obs_xyzn_params, scope_params_t *params)
{
  int shape_length = pcd_model->shape_length;

  double **R = new_matrix2(3,3);
  quaternion_to_rotation_matrix(R,q);

  double mp_pos[3];
  get_point(mp_pos, pcd_model, c_model);
  matrix_vec_mult(mp_pos, R, mp_pos, 3, 3);
  add(mp_pos, mp_pos, x, 3);

  double mp_norm[3];
  get_normal(mp_norm, pcd_model, c_model);
  matrix_vec_mult(mp_norm, R, mp_norm, 3, 3);

  double *mp_shape = pcd_model->shapes[c_model];
  
  // Look for k-NN in xyz-normal space
  double xyzn_query[6];
  mult(xyzn_query, mp_pos, params->xyz_weight, 3);
  mult(&xyzn_query[3], mp_norm, params->normal_weight, 3);
  int nn_idx[params->knn];
  double nn_d2[params->knn];
  flann_find_nearest_neighbors_index_double(obs_xyzn_index, xyzn_query, 1, nn_idx, nn_d2, params->knn, obs_xyzn_params);
  
  // then compute full feature distance on just those k-NN
  double query[shape_length + 6];
  memcpy(query, mp_shape, shape_length*sizeof(double));
  memcpy(&query[shape_length], xyzn_query, 6*sizeof(double));

  int i;
  double p[params->knn];
  for (i = 0; i < params->knn; i++) {
    double d2 = dist2(query, obs_fxyzn[nn_idx[i]], shape_length + 6);
    p[i] = exp(.5*d2 / params->f_sigma);
  }
  normalize_pmf(p, p, params->knn);
  int c_obs = nn_idx[pmfrand(p, params->knn)];

  free_matrix2(R);
 
  return c_obs;
}


/*
 * Use only least-squares information (not OLFs).
 */
void get_model_pose_distribution_from_correspondences_LS(pcd_t *pcd_obs, pcd_t *pcd_model, int *c_obs, int *c_model, int n,
							 double xyz_sigma, double *x, bingham_t *B)
{
  // shift point cloud centroids to the origin and compute translation, x
  double **points_obs = new_matrix2(n, 3);
  double **points_model = new_matrix2(n, 3);
  int i;
  for (i = 0; i < n; ++i) {
    get_point(points_obs[i], pcd_obs, c_obs[i]);
    get_point(points_model[i], pcd_model, c_model[i]);

    //printf("points_obs[%d] = [%.4f, %.4f, %.4f]\n", i, points_obs[i][0], points_obs[i][1], points_obs[i][2]);
    //printf("points_model[%d] = [%.4f, %.4f, %.4f]\n", i, points_model[i][0], points_model[i][1], points_model[i][2]);
  }
  double mean_obs[3], mean_model[3];
  mean(mean_obs, points_obs, n, 3);
  mean(mean_model, points_model, n, 3);
  sub(x, mean_obs, mean_model, 3);
  
  for (i = 0; i < n; ++i) {
    sub(points_obs[i], points_obs[i], mean_obs, 3);
    sub(points_model[i], points_model[i], mean_model, 3);
  }

  // compute LS Bingham distributions for each point correspondence
  bingham_t B_ls[n];
  for (i = 0; i < n; ++i) {
    bingham_alloc(&B_ls[i], 4);
  }
  for (i = 0; i < n; ++i) {
    double *v_obs = points_obs[i];
    double *v_model = points_model[i];
    double q_obs[4], q_model[4];
    vector_to_possible_quaternion(q_obs, v_obs);
    vector_to_possible_quaternion(q_model, v_model);
    double k = norm(v_obs, 3) * norm(v_model, 3) / (xyz_sigma * xyz_sigma);
    B->d = 4;
    B->Z[0] = -2 * k;
    B->Z[1] = -2 * k;
    B->Z[2] = 0;
    double V_data[12] = {0, 0, 1, 0,   0, 0, 0, 1,   0, 1, 0, 0};
    memcpy(B->V[0], V_data, 12*sizeof(double));
    double q_inv[4];
    quaternion_inverse(q_inv, q_model);
    bingham_pre_rotate_3d(B, B, q_inv);
    bingham_post_rotate_3d(&B_ls[i], B, q_obs);
  }  

  bingham_mult_array(B, B_ls, n, 1);
  
  for (i = 0; i < n; ++i)
    bingham_free(&B_ls[i]);

  free_matrix2(points_obs);
  free_matrix2(points_model);
}

/*
 * Compute x and p(q|x) by combining Bingham distributions from least-squares
 * alignment (LS) with Bingham distributions from OLFs.  The tuple (x,q)
 * where q is sampled from B indicates that one should rotate pcd_model(c_model)
 * about its centroid by q, then shift by x.
 */
void get_model_pose_distribution_from_correspondences(pcd_t *pcd_obs, pcd_t *pcd_model, int *c_obs, int *c_model, int n,
						      double xyz_sigma, double *x, bingham_t *B)
{
  bingham_t B_array[n+1];
  bingham_alloc(&B_array[0], 4);
  get_model_pose_distribution_from_correspondences_LS(pcd_obs, pcd_model, c_obs, c_model, n, xyz_sigma, x, &B_array[0]);

  bingham_t *B_olf = &B_array[1];
  int i;
  for (i = 0; i < n; ++i)
    bingham_alloc(&B_olf[i], 4);
  bingham_t B_feature_to_model, B_model_to_feature;
  bingham_alloc(&B_feature_to_model, 4);
  bingham_alloc(&B_model_to_feature, 4);
  double *q_feature_to_world;

  for (i = 0; i < n; ++i) {    
    olf_to_bingham(&B_feature_to_model, c_model[i], pcd_model);
    bingham_invert_3d(&B_model_to_feature, &B_feature_to_model);
    q_feature_to_world = pcd_obs->quaternions[0][c_obs[i]];
    bingham_post_rotate_3d(&B_olf[i], &B_model_to_feature, q_feature_to_world);

    // greedy check for principal curvature flips
    if (i > 1) {
      bingham_mult_array(B, B_olf, i, 0);  // compute product of B_olf[]
      double z_prod_orig = fabs(prod(B->Z, 3));
      bingham_post_rotate_3d(&B_olf[i], &B_model_to_feature, pcd_obs->quaternions[1][c_obs[i]]);
      bingham_mult_array(B, B_olf, i, 0);  // compute product of B_olf[] with B_olf[i] flipped
      double z_prod_flipped = fabs(prod(B->Z, 3));
      if (z_prod_orig > z_prod_flipped)
	bingham_post_rotate_3d(&B_olf[i], &B_model_to_feature, q_feature_to_world);  // revert to original
    }
  }  

  bingham_mult_array(B, B_array, n, 1);

  for (i = 0; i < n+1; i++)
    bingham_free(&B_array[i]);
  bingham_free(&B_feature_to_model);
  bingham_free(&B_model_to_feature);
}

void sample_model_pose(pcd_t *pcd_model, int *c_model, int c, double *x0, bingham_t *B, double *x, double *q)
{
  double mu[3] = {0, 0, 0};

  int i;
  double point[3];
  for (i = 0; i < c; ++i) {
    get_point(point, pcd_model, c_model[i]);
    add(mu, mu, point, 3);
  }
  mult(mu, mu, 1/(double)c, 3);
  
  //bingham_sample(&q, B, 1);
  bingham_mode(q, B);
  double **R = new_matrix2(3,3);
  quaternion_to_rotation_matrix(R, q);

  // x = x0 + mu - R*mu
  matrix_vec_mult(x, R, mu, 3, 3);
  sub(x, mu, x, 3);
  add(x, x0, x, 3);

  free_matrix2(R);
}

double model_pose_likelihood(pcd_t *pcd_obs, pcd_t *pcd_model, int *c_obs, int *c_model, int n, double *x, double *q, bingham_t *B,
			     double xyz_sigma, double f_sigma, double dispersion_weight, double logp_comp[4])
{
  int shape_length = pcd_model->shape_length;

  double **R = new_matrix2(3, 3);
  quaternion_to_rotation_matrix(R, q);
  double **xyz_obs = new_matrix2(n, 3);
  double **xyz_model = new_matrix2(n, 3);
  int i;
  for (i = 0; i < n; ++i) {
    get_point(xyz_obs[i], pcd_obs, c_obs[i]);
    get_point(xyz_model[i], pcd_model, c_model[i]);
  }
  
  // compute the least squares likelihood
  double logp_ls = 0.;
  for (i = 0; i < n; ++i) {
    // dx = R*xyz_model[i] + x - xyz_obs[i]
    double dx[3];
    matrix_vec_mult(dx, R, xyz_model[i], 3, 3);
    add(dx, dx, x, 3);
    sub(dx, dx, xyz_obs[i], 3);
    logp_ls += log(normpdf(norm(dx,3), 0, xyz_sigma));
  }
  logp_ls /= (double)n;
  
  // compute the OLF orientation likelihood
  double logp_olf = log(bingham_pdf(q, B));
  
  // compute FPFH likelihood
  double logp_fpfh = 0.;
  if (f_sigma > 0) {
    for (i = 0; i < n; ++i) {
      double d = dist(pcd_obs->shapes[c_obs[i]], pcd_model->shapes[c_model[i]], shape_length);
      logp_fpfh += log(normpdf(d, 0, f_sigma));
    }
    logp_fpfh /= (double)n;
  }

  // compute dispersion likelihood
  double logp_disp = 0;
  if (dispersion_weight > 0) {
    double vars[3];
    variance(vars, xyz_model, n, 3);
    logp_disp = log(dispersion_weight * sum(vars, 3));
  }

  double logp = logp_ls + logp_olf + logp_fpfh + logp_disp;

  if (logp_comp) {
    logp_comp[0] = logp_ls;
    logp_comp[1] = logp_olf;
    logp_comp[2] = logp_fpfh;
    logp_comp[3] = logp_disp;
  }
  
  free_matrix2(xyz_obs);
  free_matrix2(xyz_model);

  return logp;
}

int sample_model_correspondence_given_model_pose(pcd_t *pcd_obs, double **model_xyzn, double **model_fxyzn, scope_params_t *params,
						 struct FLANNParameters *model_xyz_params, flann_index_t model_xyz_index,
						 double *x, double *q, int c2_obs, int sample_nn, int use_f)
{
  int shape_length = pcd_obs->shape_length;

  int i;
  double q_inv[4];
  quaternion_inverse(q_inv, q);
  double **inv_R_model = new_matrix2(3, 3);
  quaternion_to_rotation_matrix(inv_R_model, q_inv);

  double xyz_obs_point2[3];
  get_point(xyz_obs_point2, pcd_obs, c2_obs);
  sub(xyz_obs_point2, xyz_obs_point2, x, 3);
  matrix_vec_mult(xyz_obs_point2, inv_R_model, xyz_obs_point2, 3, 3);

  double nxyz_obs_point2[3];
  get_normal(nxyz_obs_point2, pcd_obs, c2_obs);
  matrix_vec_mult(nxyz_obs_point2, inv_R_model, nxyz_obs_point2, 3, 3);
  
  // look for k-NN in xyz-normal space
  double xyzn_query[6];
  mult(xyzn_query, xyz_obs_point2, params->xyz_weight, 3);
  mult(&xyzn_query[3], nxyz_obs_point2, params->normal_weight, 3);
  int nn_idx[params->knn];
  double nn_d2[params->knn];
  //flann_find_nearest_neighbors_index_double(model_xyzn_index, xyzn_query, 1, nn_idx, nn_d2, params->knn, model_xyzn_params);
  flann_find_nearest_neighbors_index_double(model_xyz_index, xyz_obs_point2, 1, nn_idx, nn_d2, params->knn, model_xyz_params);
  for (i = 0; i < params->knn; i++)
    nn_d2[i] = dist2(xyzn_query, model_xyzn[nn_idx[i]], 6);
  
  if (use_f) {
    // then compute full feature distance on just those k-NN
    double query[shape_length + 6];
    memcpy(query, pcd_obs->shapes[c2_obs], shape_length * sizeof(double));
    for (i = 0; i < 3; i++) {
      query[shape_length + i] = params->xyz_weight * xyz_obs_point2[i];
      query[shape_length + 3 + i] = params->normal_weight * nxyz_obs_point2[i];
    }
    for (i = 0; i < params->knn; i++)
      nn_d2[i] = dist2(query, model_fxyzn[nn_idx[i]], shape_length + 6);
  }

  int c2_model;
  if (sample_nn)
    c2_model = nn_idx[find_min(nn_d2, params->knn)];
  else {
    double p[params->knn];
    for (i = 0; i < params->knn; i++)
      p[i] = exp(-.5*nn_d2[i] / (params->f_sigma * params->f_sigma));
    normalize_pmf(p, p, params->knn);
    c2_model = nn_idx[pmfrand(p, params->knn)];
  }

  return c2_model;
}

void align_model_icp_dense(double *x, double *q, pcd_t *pcd_model, pcd_t *pcd_obs, range_image_t *obs_range_image,
			   range_image_t *obs_fg_range_image, scope_params_t *params)
{
  int i, iter;
  int max_iter = 20;  //TODO: make this a param
  int num_icp_points = 500;  // TODO: make this a param
  double x0[3];
  bingham_t B;
  bingham_alloc(&B, 4);

  for (iter = 0; iter < max_iter; iter++) {

    // get model icp points
    int c_model[num_icp_points];
    get_validation_points(c_model, pcd_model, num_icp_points);

    // extract transformed model icp features
    double **cloud = get_sub_cloud_at_pose(pcd_model, c_model, num_icp_points, x, q);
    double **cloud_normals = get_sub_cloud_normals_rotated(pcd_model, c_model, num_icp_points, q);

    // compute visibility mask and remove hidden model points
    int vis_mask[num_icp_points];
    for (i = 0; i < num_icp_points; i++) {
      double vis_prob = compute_visibility_prob(cloud[i], cloud_normals[i], obs_range_image, params->range_sigma);
      vis_mask[i] = (vis_prob > .1);
    }
    int idx[num_icp_points];
    int n = find(idx, vis_mask, num_icp_points);
    reorder_rows(cloud, cloud, idx, n, 3);
    reorder_rows(cloud_normals, cloud_normals, idx, n, 3);
    reorderi(c_model, c_model, idx, n);

    // compute nearest neighbors
    int c_obs[n];
    double nn_d2[n];
    int search_radius = 5;  // pixels
    range_image_find_nn(c_obs, nn_d2, cloud, cloud, n, 3, pcd_obs->points, obs_fg_range_image, search_radius);

    // update model pose
    get_model_pose_distribution_from_correspondences_LS(pcd_obs, pcd_model, c_obs, c_model, n, params->xyz_sigma, x0, &B);
    sample_model_pose(pcd_model, c_model, n, x0, &B, x, q);

    free_matrix2(cloud);
    free_matrix2(cloud_normals);
  }
}

int remove_redundant_pose_samples(double **X, double **Q, double *W, int n, double x_cluster_thresh, double q_cluster_thresh)
{
  int num_samples = n;
  
  double dx2_thresh = x_cluster_thresh * x_cluster_thresh;
  double dq_thresh = q_cluster_thresh;
  int idx[num_samples];
  idx[0] = 0;
  int cnt = 1;
  int i, j;
  for (i = 1; i < num_samples; i++) {
    int unique = 1;
    for (j = 0; j < cnt; j++) {
      double dx2 = dist2(X[i], X[j], 3);
      double dq = acos(fabs(dot(Q[i], Q[j], 4)));
      if (dx2 < dx2_thresh && dq < dq_thresh) {
	unique = 0;
	break;
      }
    }
    if (unique)
      idx[cnt++] = i;
  }
  reorder(W, W, idx, cnt);
  reorder_rows(X, X, idx, cnt, 3);
  reorder_rows(Q, Q, idx, cnt, 4);

  return cnt;
}

//void sort_pose_samples(double **X, double **Q, double *W, int **C_obs, int **C_model, int **C_issift, int num_samples, int num_correspondences)
void sort_pose_samples(double **X, double **Q, double *W, int **C_obs, int **C_model, int num_samples, int num_correspondences)
{
  int idx[num_samples];
  sort_indices(W, idx, num_samples);  // smallest to biggest
  reversei(idx, idx, num_samples);  // reverse idx (biggest to smallest)

  reorder(W, W, idx, num_samples);
  reorder_rows(X, X, idx, num_samples, 3);
  reorder_rows(Q, Q, idx, num_samples, 4);
  reorder_rowsi(C_obs, C_obs, idx, num_samples, num_correspondences);
  reorder_rowsi(C_model, C_model, idx, num_samples, num_correspondences);
  //reorder_rowsi(C_issift, C_issift, idx, num_samples, num_correspondences);
}


/*
 * Single Cluttered Object Pose Estimation (SCOPE)
 */
olf_pose_samples_t *scope(olf_model_t *model, olf_obs_t *obs, scope_params_t *params, short have_true_pose, simple_pose_t *true_pose)
{
  const double MIN_SCORE = -100000.0;

  params->verbose = 0;

  /*dbug
  params->num_samples_init = 1;
  params->num_samples = 1;
  params->num_correspondences = 2;
  params->branching_factor = 1;
  params->do_icp = 0;
  */

  int i,j;

  printf("scope()\n");

  // unpack model arguments
  pcd_t *pcd_model = model->obj_pcd;
  pcd_t *sift_model = model->sift_pcd;
  int shape_length = pcd_model->shape_length;

  // unpack obs arguments
  pcd_t *pcd_obs = obs->fg_pcd;
  pcd_t *sift_obs = obs->sift_pcd;
  pcd_t *pcd_obs_bg = obs->bg_pcd;

  // compute range images
  range_image_t *obs_range_image = pcd_to_range_image(pcd_obs_bg, 0, M_PI/1000.0);
  range_image_t *obs_fg_range_image = pcd_to_range_image_from_template(pcd_obs, obs_range_image);

  // compute model feature saliency
  double *model_pmf = compute_model_saliency(pcd_model);
  double model_cmf[pcd_model->num_points];
  cumsum(model_cmf, model_pmf, pcd_model->num_points);

  // find sift matches
  int sift_match_obs_idx[sift_obs->num_points], sift_match_model_idx[sift_obs->num_points];
  int num_sift_matches = 0; //find_obs_sift_matches(sift_match_obs_idx, sift_match_model_idx, sift_obs, sift_model, params->sift_dthresh);

  // get combined feature matrices
  double **obs_xyzn = get_xyzn_features(pcd_obs->points, pcd_obs->normals, pcd_obs->num_points, params);
  double **obs_bg_xyzn = get_xyzn_features(pcd_obs_bg->points, pcd_obs_bg->normals, pcd_obs_bg->num_points, params);
  double **model_xyzn = get_xyzn_features(pcd_model->points, pcd_model->normals, pcd_model->num_points, params);
  double **obs_fxyzn = get_fxyzn_features(pcd_obs->shapes, pcd_obs->points, pcd_obs->normals, pcd_obs->num_points, pcd_obs->shape_length, params);
  double **model_fxyzn = get_fxyzn_features(pcd_model->shapes, pcd_model->points, pcd_model->normals, pcd_model->num_points, pcd_model->shape_length, params);
  float **obs_fsurf = get_fsurf_features(pcd_obs->shapes, pcd_obs->sdw, pcd_obs->num_points, pcd_obs->shape_length, params);
  float **model_fsurf = get_fsurf_features(pcd_model->shapes, pcd_model->sdw, pcd_model->num_points, pcd_model->shape_length, params);

  // flann params
  struct FLANNParameters flann_params_single = DEFAULT_FLANN_PARAMETERS;
  flann_params_single.algorithm = FLANN_INDEX_KDTREE_SINGLE;
  struct FLANNParameters flann_params = DEFAULT_FLANN_PARAMETERS;
  flann_params.algorithm = FLANN_INDEX_KDTREE;
  //flann_params.trees = 8;
  //flann_params.checks = 64;
  struct FLANNParameters obs_xyzn_params = flann_params_single;
  struct FLANNParameters model_xyz_params = flann_params_single;
  //struct FLANNParameters model_xyzn_params = flann_params_single;
  struct FLANNParameters model_fsurf_params = flann_params;

  printf("Building FLANN indices\n");

  // build flann indices
  float speedup;
  flann_index_t obs_xyzn_index = flann_build_index_double(obs_xyzn[0], pcd_obs->num_points, 6, &speedup, &obs_xyzn_params);
  flann_index_t model_xyz_index = flann_build_index_double(pcd_model->points[0], pcd_model->num_points, 3, &speedup, &model_xyz_params);
  //flann_index_t model_xyzn_index = flann_build_index_double(model_xyzn[0], pcd_model->num_points, 6, &speedup, &model_xyzn_params);
  flann_index_t model_fsurf_index = flann_build_index_float(model_fsurf[0], pcd_model->num_points, shape_length + 2, &speedup, &model_fsurf_params);

  //dbug
  double t0_scope = get_time_ms();

  // create correspondences and pose data structures
  int num_samples_init = params->num_samples_init + num_sift_matches;
  int num_samples = params->num_samples;
  int branching_factor = params->branching_factor;
  int max_num_samples = MAX(num_samples_init, num_samples * branching_factor);
  int **C_obs = new_matrix2i(max_num_samples, params->num_correspondences);     // obs correspondences
  int **C_model = new_matrix2i(max_num_samples, params->num_correspondences);   // model correspondences
  //int **C_issift = new_matrix2i(max_num_samples, params->num_correspondences);  // sift indicator
  double **X = new_matrix2(max_num_samples, 3);                                 // sample positions
  double **Q = new_matrix2(max_num_samples, 4);                                 // sample orientations
  double W[max_num_samples];                                                    // sample weights

  printf("Sampling first correspondences\n");

  double t0 = get_time_ms();

  // sample poses from one correspondence
  for (i = 0; i < num_samples_init; i++) {
    if (i < num_sift_matches) {  // sift correspondence
      //C_issift[i][0] = 1;
      C_obs[i][0] = sift_match_obs_idx[i];
      C_model[i][0] = sift_match_model_idx[i];

      // sample a model pose given one correspondence
      model_pose_from_one_correspondence(X[i], Q[i], C_obs[i][0], C_model[i][0], sift_obs, sift_model);
    }
    else {  // fpfh+sdw correspondence
      //C_issift[i][0] = 0;

      C_obs[i][0] = irand(pcd_obs->num_points);
      int nn_idx[params->knn];
      float nn_d2[params->knn];
      flann_find_nearest_neighbors_index_float(model_fsurf_index, obs_fsurf[C_obs[i][0]], 1, nn_idx, nn_d2, params->knn, &model_fsurf_params);
      double p[params->knn];
      for (j = 0; j < params->knn; j++)
	p[j] = exp(-.5*nn_d2[j] / (params->fsurf_sigma * params->fsurf_sigma));
      normalize_pmf(p, p, params->knn);
      j = pmfrand(p, params->knn);
      C_model[i][0] = nn_idx[j];

      // sample a model pose given one correspondence
      model_pose_from_one_correspondence(X[i], Q[i], C_obs[i][0], C_model[i][0], pcd_obs, pcd_model);
    }
  }

  //dbug
  printf("Sampled c=1 poses in %.3f seconds\n", (get_time_ms() - t0) / 1000.0);  //dbug
  if (have_true_pose)
    print_good_poses(true_pose, X, Q, num_samples_init);
  t0 = get_time_ms();

  // score hypotheses
  for (i = 0; i < num_samples_init; i++)
    W[i] = model_placement_score(X[i], Q[i], pcd_model, pcd_obs_bg, obs_range_image, obs_bg_xyzn, params);

  // sort hypotheses
  //sort_pose_samples(X, Q, W, C_obs, C_model, C_issift, num_samples_init, params->num_correspondences);
  sort_pose_samples(X, Q, W, C_obs, C_model, num_samples_init, params->num_correspondences);

  //dbug
  printf("Scored and sorted c=1 poses in %.3f seconds\n", (get_time_ms() - t0) / 1000.0);  //dbug
  if (have_true_pose)
    print_good_poses(true_pose, X, Q, num_samples);

  int c;
  int num_correspondences = params->num_correspondences;
  for (c = 1; c < num_correspondences; ++c) {

    //dbug
    printf("c = %d\n", c);
    t0 = get_time_ms();

    // branch
    for (i = 1; i < branching_factor; i++)
      memcpy(&W[i*num_samples], W, num_samples * sizeof(double));
    repmat(X, X, branching_factor, 1, num_samples, 3);
    repmat(Q, Q, branching_factor, 1, num_samples, 4);
    repmati(C_obs, C_obs, branching_factor, 1, num_samples, num_correspondences);
    repmati(C_model, C_model, branching_factor, 1, num_samples, num_correspondences);
    //repmati(C_issift, C_issift, branching_factor, 1, num_samples, num_correspondences);

    for (i = 0; i < num_samples * branching_factor; ++i) {

      // sample a new model point
      C_model[i][c] = sample_model_point_given_model_pose(X[i], Q[i], C_model[i], c, model_cmf, pcd_model);  // + .02 ms

      if (C_model[i][c] < 0)
	continue;

      // sample a corresponding obs point
      C_obs[i][c] = sample_obs_correspondence_given_model_pose(X[i], Q[i], C_model[i][c], pcd_model, obs_fxyzn, obs_xyzn_index, &obs_xyzn_params, params);  // + .18 ms

      // compute max likelihood model pose given the point correspondences
      double x0[3] = {0,0,0};
      bingham_t B;
      bingham_alloc(&B, 4);
      get_model_pose_distribution_from_correspondences(pcd_obs, pcd_model, C_obs[i], C_model[i], c+1, params->xyz_sigma, x0, &B);    // + .04 ms
      sample_model_pose(pcd_model, C_model[i], c+1, x0, &B, X[i], Q[i]);

      if (params->do_icp) {  // + .5 ms

	int c_model_new[c];
	double x_new[3], q_new[3];
	double logp, logp_new;
	double x0_new[3] = {0,0,0};
	bingham_t B_new;
	bingham_alloc(&B_new, 4);
	//bingham_new_uniform(&B_new, 4);
	//B_new.V[0][3] = B_new.V[1][1] = B_new.V[2][2] = 1;

	logp = model_pose_likelihood(pcd_obs, pcd_model, C_obs[i], C_model[i], c+1, X[i], Q[i], &B, params->xyz_sigma, -1, -1, NULL);

	int k;
	for (k = 0; k < 20; ++k) {
	  int j;
	  for (j = 0; j <= c; ++j)
	    c_model_new[j] = sample_model_correspondence_given_model_pose(pcd_obs, model_xyzn, model_fxyzn, params, &model_xyz_params, model_xyz_index,
									  X[i], Q[i], C_obs[i][j], 1, 0);  //sample nn, no fpfh
	  get_model_pose_distribution_from_correspondences(pcd_obs, pcd_model, C_obs[i], c_model_new, c+1, params->xyz_sigma, x0_new, &B_new);
	  sample_model_pose(pcd_model, c_model_new, c+1, x0_new, &B_new, x_new, q_new);
	  logp_new = model_pose_likelihood(pcd_obs, pcd_model, C_obs[i], c_model_new, c+1, x_new, q_new, &B_new, params->xyz_sigma, -1, -1, NULL);
	  
	  if (logp_new > logp) {
	    memcpy(C_model[i], c_model_new, (c+1)*sizeof(int));
	    memcpy(X[i], x_new, 3*sizeof(double));
	    memcpy(Q[i], q_new, 4*sizeof(double));
	    bingham_free(&B);
	    bingham_copy(&B, &B_new);
	    logp = logp_new;
	  }
	  else  // stop ICP when model pose likelihood doesn't increase anymore
	    break;
	}
      }
    }
    
    printf("Sampled c=%d poses in %.3f seconds\n", c+1, (get_time_ms() - t0) / 1000.0);  //dbug

    t0 = get_time_ms();

    // score hypotheses
    for (i = 0; i < num_samples * branching_factor; i++) {
      if (C_model[i][c] >= 0)
	W[i] = model_placement_score(X[i], Q[i], pcd_model, pcd_obs_bg, obs_range_image, obs_bg_xyzn, params);  // + .56 ms
      else
	W[i] = MIN_SCORE;
    }

    // sort samples by weight and prune branches
    //sort_pose_samples(X, Q, W, C_obs, C_model, C_issift, num_samples * branching_factor, params->num_correspondences);
    sort_pose_samples(X, Q, W, C_obs, C_model, num_samples * branching_factor, params->num_correspondences);

    //dbug
    for (i = 0; i < num_samples; i++) {
      if (W[i] == MIN_SCORE) {
	printf("WARNING: W[%d] == MIN_SCORE --> Setting num_samples = %d\n", i, i);
	num_samples = i;
      }
    }

    //dbug
    printf("Scored and sorted c=%d poses in %.3f seconds\n", c+1, (get_time_ms() - t0) / 1000.0);  //dbug
    if (have_true_pose)
      print_good_poses(true_pose, X, Q, num_samples);
  }

  //dbug
  t0 = get_time_ms();

  // params
  double w_sigma = 1.0;  //TODO: make this a parameter
  int num_score_trials = 10;
  double w_sigma_multi = w_sigma / sqrt((double)num_score_trials);

  // remove low-weight samples, cluster poses, re-weight, sort, and remove low-weight samples again
  num_samples = find_first_lt(W, W[0] - 2*w_sigma, num_samples);
  if (params->pose_clustering)
    num_samples = remove_redundant_pose_samples(X, Q, W, num_samples, params->x_cluster_thresh, params->q_cluster_thresh);
  for (i = 0; i < num_samples; i++)
    W[i] = model_placement_score_multi(X[i], Q[i], pcd_model, pcd_obs_bg, obs_range_image, obs_bg_xyzn, params, num_score_trials);
  sort_pose_samples(X, Q, W, C_obs, C_model, num_samples, params->num_correspondences);
  num_samples = find_first_lt(W, W[0] - 2*w_sigma_multi, num_samples);

  // align samples, cluster poses, re-weight, sort, and remove low-weight samples
  if (params->do_final_icp) {
    for (i = 0; i < num_samples; i++)
      align_model_icp_dense(X[i], Q[i], pcd_model, pcd_obs, obs_range_image, obs_fg_range_image, params);
    if (params->pose_clustering)
      num_samples = remove_redundant_pose_samples(X, Q, W, num_samples, params->x_cluster_thresh, params->q_cluster_thresh);
    for (i = 0; i < num_samples; i++)
      W[i] = model_placement_score_multi(X[i], Q[i], pcd_model, pcd_obs_bg, obs_range_image, obs_bg_xyzn, params, num_score_trials);
    sort_pose_samples(X, Q, W, C_obs, C_model, num_samples, params->num_correspondences);
    num_samples = find_first_lt(W, W[0] - 2*w_sigma_multi, num_samples);
  }

  // pack return value
  olf_pose_samples_t *pose_samples;
  safe_calloc(pose_samples, 1, olf_pose_samples_t);
  pose_samples->X = X;
  pose_samples->Q = Q;
  pose_samples->W = W;
  pose_samples->n = num_samples;

  //dbug
  printf("Scored and sorted final poses in %.3f seconds\n", (get_time_ms() - t0) / 1000.0);
  if (have_true_pose)
    print_good_poses_verbose(true_pose, X, Q, W, w_sigma_multi, num_samples);

  //dbug
  params->verbose = 1;
  pose_samples->vis_probs = new_matrix2(num_samples, pcd_model->num_points);
  for (i = 0; i < num_samples; i++) {
    printf("\nsample %d:\n", i);
    params->num_validation_points = 0;
    model_placement_score(X[i], Q[i], pcd_model, pcd_obs_bg, obs_range_image, obs_bg_xyzn, params);
    memcpy(pose_samples->vis_probs[i], mps_vis_prob_, pcd_model->num_points * sizeof(double));
    //model_placement_score_multi(X[i], Q[i], pcd_model, pcd_obs_bg, obs_range_image, obs_bg_xyzn, params, num_score_trials);
  }

  // cleanup
  free(model_pmf);
  free_matrix2(obs_xyzn);
  free_matrix2(model_xyzn);
  free_matrix2(obs_fxyzn);
  free_matrix2(model_fxyzn);
  free_matrix2f(obs_fsurf);
  free_matrix2f(model_fsurf);
  flann_free_index(obs_xyzn_index, &obs_xyzn_params);
  flann_free_index(model_xyz_index, &model_xyz_params);
  //flann_free_index(model_xyzn_index, &model_xyzn_params);
  flann_free_index(model_fsurf_index, &model_fsurf_params);
  free_matrix2i(C_obs);
  free_matrix2i(C_model);
  //free_matrix2i(C_issift);
  
  //dbug
  printf("Ran scope() in %.3f seconds\n", (get_time_ms() - t0_scope) / 1000.0);  //dbug

  return pose_samples;
}













//-------------------------- DEPRECATED --------------------------//


/*
 * add balls model to a pcd
 *
static void pcd_add_balls(pcd_t *pcd)
{
  int i, s, b;
  int ch_balls = pcd_channel(pcd, "balls");
  int ch_segments = pcd_channel(pcd, "segments");

  if (ch_balls < 0)
    return;

  // create pcd_balls_t
  safe_calloc(pcd->balls, 1, pcd_balls_t);
  pcd_balls_t *B = pcd->balls;

  // get ball labels
  safe_calloc(B->ball_labels, pcd->num_points, int);
  for (i = 0; i < pcd->num_points; i++)
    B->ball_labels[i] = (int)(pcd->data[ch_balls][i]);

  // get segment labels
  safe_calloc(B->segment_labels, pcd->num_points, int);
  if (ch_segments >= 0)
    for (i = 0; i < pcd->num_points; i++)
      B->segment_labels[i] = (int)(pcd->data[ch_segments][i]);

  // get num segments
  B->num_segments = imax(B->segment_labels, pcd->num_points) + 1;

  ***
  // compute segment centers
  B->segment_centers = new_matrix2(B->num_segments, 3);
  int segment_cnts[B->num_segments];
  memset(segment_cnts, 0, B->num_segments * sizeof(int));
  for (i = 0; i < pcd->num_points; i++) {
    s = B->segment_labels[i];
    B->segment_centers[s][0] += pcd->points[0][i];
    B->segment_centers[s][1] += pcd->points[1][i];
    B->segment_centers[s][2] += pcd->points[2][i];
    segment_cnts[s]++;
  }
  for (s = 0; s < B->num_segments; s++)
    mult(B->segment_centers[s], B->segment_centers[s], 1.0/(double)segment_cnts[s], 3);

  // compute segment radii
  safe_calloc(B->segment_radii, B->num_segments, double);
  for (i = 0; i < pcd->num_points; i++) {
    s = B->segment_labels[i];
    double dx = pcd->points[0][i] - B->segment_centers[s][0];
    double dy = pcd->points[1][i] - B->segment_centers[s][1];
    double dz = pcd->points[2][i] - B->segment_centers[s][2];
    B->segment_radii[s] += dx*dx + dy*dy + dz*dz;
  }
  for (s = 0; s < B->num_segments; s++)
    B->segment_radii[s] = sqrt(B->segment_radii[s] / (double)segment_cnts[s]);

  // compute mean segment radius
  B->mean_segment_radius = sum(B->segment_radii, B->num_segments);
  B->mean_segment_radius /= (double)B->num_segments;
  ***

  // get num balls
  safe_calloc(B->num_balls, B->num_segments, int);
  for (i = 0; i < pcd->num_points; i++) {
    s = B->segment_labels[i];
    b = B->ball_labels[i];
    if (B->num_balls[s] < b+1)
      B->num_balls[s] = b+1;
  }

  // compute ball centers
  safe_calloc(B->ball_centers, B->num_segments, double**);
  for (s = 0; s < B->num_segments; s++)
    B->ball_centers[s] = new_matrix2(B->num_balls[s], 3);
  int max_balls = imax(B->num_balls, B->num_segments);
  int ball_cnts[B->num_segments][max_balls];
  memset(ball_cnts, 0, B->num_segments * max_balls * sizeof(int));
  for (i = 0; i < pcd->num_points; i++) {
    s = B->segment_labels[i];
    b = B->ball_labels[i];
    B->ball_centers[s][b][0] += pcd->points[0][i];
    B->ball_centers[s][b][1] += pcd->points[1][i];
    B->ball_centers[s][b][2] += pcd->points[2][i];
    ball_cnts[s][b]++;
  }
  for (s = 0; s < B->num_segments; s++)
    for (b = 0; b < B->num_balls[s]; b++)
      mult(B->ball_centers[s][b], B->ball_centers[s][b], 1.0/(double)ball_cnts[s][b], 3);

  // compute ball radii
  B->ball_radii = new_matrix2(B->num_segments, max_balls);
  for (i = 0; i < pcd->num_points; i++) {
    s = B->segment_labels[i];
    b = B->ball_labels[i];
    double dx = B->ball_centers[s][b][0] - pcd->points[0][i];
    double dy = B->ball_centers[s][b][1] - pcd->points[1][i];
    double dz = B->ball_centers[s][b][2] - pcd->points[2][i];
    B->ball_radii[s][b] += dx*dx + dy*dy + dz*dz;
  }
  for (s = 0; s < B->num_segments; s++)
    for (b = 0; b < B->num_balls[s]; b++)
      B->ball_radii[s][b] = sqrt(B->ball_radii[s][b] / (double)ball_cnts[s][b]);

  // compute mean ball radius
  int nballs = 0;
  for (s = 0; s < B->num_segments; s++) {
    for (b = 0; b < B->num_balls[s]; b++) {
      if (ball_cnts[s][b] > 0) {
	B->mean_ball_radius += B->ball_radii[s][b];
	nballs++;
      }
    }
  }
  B->mean_ball_radius /= (double)nballs;

  // compute segment centers & radii
  B->segment_centers = new_matrix2(B->num_segments, 3);
  safe_calloc(B->segment_radii, B->num_segments, double);
  for (s = 0; s < B->num_segments; s++) {
    // compute center
    nballs = 0;
    for (b = 0; b < B->num_balls[s]; b++) {
      if (ball_cnts[s][b] > 0) {
	nballs++;
	add(B->segment_centers[s], B->segment_centers[s], B->ball_centers[s][b], 3);
      }
    }
    mult(B->segment_centers[s], B->segment_centers[s], 1/(double)nballs, 3);
    // compute radius
    for (b = 0; b < B->num_balls[s]; b++) {
      if (ball_cnts[s][b] > 0) {
	double d = dist(B->segment_centers[s], B->ball_centers[s][b], 3);
	double r = B->ball_radii[s][b];
	if (B->segment_radii[s] < d + r)
	  B->segment_radii[s] = d + r;
      }
    }
  }

  // compute mean segment radius
  B->mean_segment_radius = sum(B->segment_radii, B->num_segments);
  B->mean_segment_radius /= (double)B->num_segments;
}
*/

/*
 * free pcd->balls
 *
static void pcd_free_balls(pcd_t *pcd)
{
  if (pcd->balls == NULL)
    return;

  pcd_balls_t *B = pcd->balls;
  int i;

  if (B->num_balls)
    free(B->num_balls);
  if (B->segment_labels)
    free(B->segment_labels);
  if (B->ball_labels)
    free(B->ball_labels);
  if (B->segment_centers)
    free_matrix2(B->segment_centers);
  if (B->segment_radii)
    free(B->segment_radii);
  if (B->ball_centers) {
    for (i = 0; i < B->num_segments; i++)
      free_matrix2(B->ball_centers[i]);
    free(B->ball_centers);
  }
  if (B->ball_radii)
    free_matrix2(B->ball_radii);

  free(pcd->balls);
}
*/


/*
 * check if a pcd has all the channels needed to biuld an OLF model
 *
static int pcd_has_olf_channels(pcd_t *pcd)
{
  return (pcd_channel(pcd, "x")>=0 && pcd_channel(pcd, "y")>=0 && pcd_channel(pcd, "z")>=0 &&
	  pcd_channel(pcd, "nx")>=0 && pcd_channel(pcd, "ny")>=0 && pcd_channel(pcd, "nz")>=0 &&
	  pcd_channel(pcd, "pcx")>=0 && pcd_channel(pcd, "pcy")>=0 && pcd_channel(pcd, "pcz")>=0 &&
	  pcd_channel(pcd, "pc1")>=0 && pcd_channel(pcd, "pc2")>=0 && pcd_channel(pcd, "cluster")>=0 &&
	  pcd_channel(pcd, "f1")>=0 && pcd_channel(pcd, "f33")>=0);
}
*/

/*
static void pcd_random_walk(int *I, pcd_t *pcd, int i0, int n, double sigma)
{
  //dbug
  double **X = new_matrix2(pcd->num_points, 3);
  transpose(X, pcd->points, 3, pcd->num_points);

  I[0] = i0;
  int cnt, i = i0;
  double x[3];
  for (cnt = 1; cnt < n; cnt++) {
    x[0] = normrand(pcd->points[0][i], sigma);
    x[1] = normrand(pcd->points[1][i], sigma);
    x[2] = normrand(pcd->points[2][i], sigma);

    i = kdtree_NN(pcd->points_kdtree, x);
    I[cnt] = i;

    fprintf(stderr, "random walk step = %f\n", dist(X[I[cnt]], X[I[cnt-1]], 3)); //dbug
  }
}
*/

/*
 * (fast, randomized) intersection of two pcds with a balls model --
 * computes a list of point indices that (approximately) intersect with the model balls,
 * returns the number of intersecting points
 *
static int pcd_intersect(int *idx, pcd_t *pcd, pcd_t *model, double *x, double *q)
{
  int i, s, b;
  int num_points = pcd->num_points;
  int num_model_balls = model->balls->num_balls[0];
  int num_segments = pcd->balls->num_segments;
  int *num_segment_balls = pcd->balls->num_balls;
  double model_radius = model->balls->segment_radii[0];
  double *model_ball_radii = model->balls->ball_radii[0];
  double **segment_centers = pcd->balls->segment_centers;
  double *segment_radii = pcd->balls->segment_radii;
  double ***segment_ball_centers = pcd->balls->ball_centers;
  double **segment_ball_radii = pcd->balls->ball_radii;

  // apply transform (x,q) to model
  double **R = new_matrix2(3,3);
  quaternion_to_rotation_matrix(R,q);
  double model_center[3];
  matrix_vec_mult(model_center, R, model->balls->segment_centers[0], 3, 3);
  add(model_center, model_center, x, 3);
  double **model_ball_centers = new_matrix2(num_model_balls, 3);
  for (b = 0; b < num_model_balls; b++) {
    matrix_vec_mult(model_ball_centers[b], R, model->balls->ball_centers[0][b], 3, 3);
    add(model_ball_centers[b], model_ball_centers[b], x, 3);
  }
  free_matrix2(R);

  // compute close segments
  int close_segments[num_segments];
  memset(close_segments, 0, num_segments * sizeof(int));
  for (s = 0; s < num_segments; s++) {
    //printf("break 1\n");
    if (dist(segment_centers[s], model_center, 3) < segment_radii[s] + model_radius) {  // segment intersects model
      for (b = 0; b < num_model_balls; b++) {
	//printf("break 2\n");
	if (dist(segment_centers[s], model_ball_centers[b], 3) < segment_radii[s] + model_ball_radii[b]) {  // segment intersects model ball
	  double p = 0;
	  for (i = 0; i < num_segment_balls[s]; i++) {
	    if (segment_ball_radii[s][i] > 0.0) {
	      //printf("break 3\n");
	      if (dist(segment_ball_centers[s][i], model_ball_centers[b], 3) < segment_ball_radii[s][i] + model_ball_radii[b])  // segment ball intersects model ball
		p += 1/(double)num_segment_balls[s];
	    }
	  }
	  if (frand() < p)
	    close_segments[s] = 1;
	}
      }
    }
  }
  free_matrix2(model_ball_centers);

  // compute point indices
  int n = 0;
  for (i = 0; i < num_points; i++) {
    s = pcd->balls->segment_labels[i];
    if (close_segments[s])
      idx[n++] = i;
  }

  return n;
}
*/


/*
 * loads an olf from fname.pcd and fname.bmx
 *
olf_t *load_olf(char *fname)
{
  int i;
  char f[1024];

  //dbug
  //double t;
  //t = get_time_ms();

  // load pcd
  sprintf(f, "%s.pcd", fname);
  pcd_t *pcd = load_pcd(f);
  if (pcd == NULL)
    return NULL;
  if (!pcd_has_olf_channels(pcd)) {
    fprintf(stderr, "Warning: pcd doesn't have olf channels!\n");
    pcd_free(pcd);
    free(pcd);
    return NULL;
  }

  //dbug
  //fprintf(stderr, "Loaded olf pcd in %f ms\n", get_time_ms() - t);
  //t = get_time_ms();  

  // load bmx
  sprintf(f, "%s.bmx", fname);
  int num_clusters;
  bingham_mix_t *bmx = load_bmx(f, &num_clusters);
  if (bmx == NULL) {
    pcd_free(pcd);
    free(pcd);
    return NULL;
  }

  //dbug
  //fprintf(stderr, "Loaded olf bmx in %f ms\n", get_time_ms() - t);
  //t = get_time_ms();  

  // create olf
  olf_t *olf;
  safe_calloc(olf, 1, olf_t);
  olf->pcd = pcd;
  olf->bmx = bmx;
  olf->num_clusters = num_clusters;

  // get shape descriptor length
  olf->shape_length = 33;

  // create temporary shape matrix
  double **S = new_matrix2(pcd->num_points, olf->shape_length);
  transpose(S, pcd->shapes, olf->shape_length, pcd->num_points);

  // get cluster weights
  safe_calloc(olf->cluster_weights, num_clusters, double);
  for (i = 0; i < pcd->num_points; i++) {
    int c = pcd->clusters[i];
    olf->cluster_weights[c]++;
  }
  mult(olf->cluster_weights, olf->cluster_weights, 1/(double)pcd->num_points, num_clusters);

  // get mean shapes
  olf->mean_shapes = new_matrix2(num_clusters, olf->shape_length);
  for (i = 0; i < pcd->num_points; i++) {
    int c = pcd->clusters[i];
    add(olf->mean_shapes[c], olf->mean_shapes[c], S[i], olf->shape_length);
  }
  for (i = 0; i < num_clusters; i++) {
    double cluster_size = olf->cluster_weights[i] * pcd->num_points;
    mult(olf->mean_shapes[i], olf->mean_shapes[i], 1/cluster_size, olf->shape_length);
  }

  // get shape variances
  safe_calloc(olf->shape_variances, num_clusters, double);
  for (i = 0; i < pcd->num_points; i++) {
    int c = pcd->clusters[i];
    olf->shape_variances[c] += dist2(S[i], olf->mean_shapes[c], olf->shape_length);
  }
  for (i = 0; i < num_clusters; i++) {
    double cluster_size = olf->cluster_weights[i] * pcd->num_points;
    olf->shape_variances[i] /= cluster_size;
  }

  free_matrix2(S);

  //dbug
  //fprintf(stderr, "Computed cluster shapes in %f ms\n", get_time_ms() - t);
  //t = get_time_ms();  

  // load hll models
  sprintf(f, "%s.hll", fname);
  int num_hll;
  olf->hll = load_hlls(f, &num_hll);

  if (olf->hll == NULL) {

    // create hll models
    safe_calloc(olf->hll, num_clusters, hll_t);
    int c;
    for (c = 0; c < num_clusters; c++) {
      int n = (int)round(olf->cluster_weights[c] * pcd->num_points);
      double **Q = new_matrix2(2*n, 4);
      double **X = new_matrix2(2*n, 3);
      int cnt=0;
      for (i = 0; i < pcd->num_points; i++) {
	if (pcd->clusters[i] == c) {
	  memcpy(Q[cnt], pcd->quaternions[0][i], 4*sizeof(double));
	  memcpy(Q[cnt+1], pcd->quaternions[1][i], 4*sizeof(double));
	  X[cnt][0] = X[cnt+1][0] = pcd->points[0][i];
	  X[cnt][1] = X[cnt+1][1] = pcd->points[1][i];
	  X[cnt][2] = X[cnt+1][2] = pcd->points[2][i];
	  cnt += 2;
	}
      }
      hll_new(&olf->hll[c], Q, X, 2*n, 4, 3);
      hll_cache(&olf->hll[c], Q, 2*n);
    }

    // save hll models
    save_hlls(f, olf->hll, num_clusters);
  }

  //dbug
  //fprintf(stderr, "Created HLL in %f ms\n", get_time_ms() - t);
  //t = get_time_ms();

  // set olf params (TODO--load this from a .olf file)
  olf->rot_symm = 0;
  olf->num_validators = 5;
  olf->lambda = 1; //.5;
  olf->pose_agg_x = .05;  // meters
  olf->pose_agg_q = .2;  // radians

  return olf;
}
*/

/*
 * frees the contents of an olf_t, but not the pointer itself
 *
void olf_free(olf_t *olf)
{
  int i;

  if (olf->pcd)
    pcd_free(olf->pcd);
  if (olf->bmx) {
    for (i = 0; i < olf->num_clusters; i++)
      bingham_mixture_free(&olf->bmx[i]);
    free(olf->bmx);
  }
  if (olf->cluster_weights)
    free(olf->cluster_weights);
  if (olf->mean_shapes)
    free_matrix2(olf->mean_shapes);
  if (olf->shape_variances)
    free(olf->shape_variances);
}
*/

/*
 * classify pcd points (add channel "cluster") using olf shapes
 *
void olf_classify_points(pcd_t *pcd, olf_t *olf)
{
  int ch = pcd_channel(pcd, "cluster");
  if (ch < 0) {
    ch = pcd_add_channel(pcd, "cluster");
    safe_malloc(pcd->clusters, pcd->num_points, int);
  }

  // create temporary shape matrix
  double **S = new_matrix2(pcd->num_points, olf->shape_length);
  transpose(S, pcd->shapes, olf->shape_length, pcd->num_points);

  int i, j;

  printf("shape stdev = [");
  double sigma = 0.0;
  for (i = 0; i < olf->num_clusters; i++) {
    sigma += sqrt(olf->shape_variances[i]);
    printf("%f ", sqrt(olf->shape_variances[i]));
  }
  printf("]\n");
  sigma /= (double) olf->num_clusters;

  double d, jmin=0;
  //double dmin;
  double p[olf->num_clusters];
  for (i = 0; i < pcd->num_points; i++) {
    //dmin = DBL_MAX;
    for (j = 0; j < olf->num_clusters; j++) {
      d = dist(S[i], olf->mean_shapes[j], olf->shape_length);
      p[j] = normpdf(d, 0, sigma/10.0);
      //if (d < dmin) {
      //  dmin = d;
      //  jmin = j;
      //}
    }
    mult(p, p, 1/sum(p, olf->num_clusters), olf->num_clusters);
    jmin = pmfrand(p, olf->num_clusters);
    pcd->clusters[i] = jmin;
    pcd->data[ch][i] = (double)jmin;
  }

  free_matrix2(S);
}
*/

/*
 * computes the pdf of pose (x,q) given n points from pcd w.r.t. olf,
 * assumes that pcd has channel "cluster" (i.e. points are already classified)
 *
double olf_pose_pdf(double *x, double *q, olf_t *olf, pcd_t *pcd, int *indices, int n)
{
  int i;

  //dbug
  //printf("x = %f, %f, %f\n", x[0], x[1], x[2]);
  //printf("q = %f, %f, %f, %f\n", q[0], q[1], q[2], q[3]);

  // multi-feature likelihood
  if (n > 1) {
    double logp = 0;
    for (i = 0; i < n; i++)
      logp += log(olf_pose_pdf(x, q, olf, pcd, &indices[i], 1));

    return olf->lambda * exp(olf->lambda*logp/(double)n);
  }

  i = indices[0];  // validation point index

  double *x_world_to_model = x;
  double *q_world_to_model = q;
  double q_model_to_world[4];  //q_inv[4];

  quaternion_inverse(q_model_to_world, q_world_to_model);  //(q_inv, q);

  double x_feature[3] = {pcd->points[0][i], pcd->points[1][i], pcd->points[2][i]};
  double *q_feature = (frand() < .5 ? pcd->quaternions[0][i] : pcd->quaternions[1][i]);
  //double *q_feature = pcd->quaternions[0][i]; //dbug


  *** dbug
  double x_dbug[3] = {1000,0,0};    // world to model
  double q_dbug[4] = {0,1,0,0};  // world to model
  double q_feature_dbug[4];
  quaternion_mult(q_feature_dbug, q_feature, q_dbug);
  q_feature = q_feature_dbug;
  double **R_dbug = new_matrix2(3,3);
  quaternion_to_rotation_matrix(R_dbug, q_dbug);
  double x_feature_dbug[3];
  matrix_vec_mult(x_feature_dbug, R_dbug, x_feature, 3, 3);
  add(x_feature, x_feature_dbug, x_dbug, 3);
  free_matrix2(R_dbug);
  ***

  // q2: rotation from model -> feature
  double q_model_to_feature[4];  //q2[4];
  double *q_model_to_feature_ptr[1];  // *q2_ptr[1];
  q_model_to_feature_ptr[0] = q_model_to_feature;  //q2_ptr[0] = q2;
  quaternion_mult(q_model_to_feature, q_feature, q_model_to_world);  //(q2, q_feature, q_inv);

  // x2: translation from model -> feature
  double xi[3];
  sub(xi, x_feature, x_world_to_model, 3);
  double **R_model_to_world = new_matrix2(3,3);  //R_inv
  quaternion_to_rotation_matrix(R_model_to_world, q_model_to_world);  //(R_inv, q_inv);
  double x_model_to_feature[3];   // x2 = R_inv*xi
  matrix_vec_mult(x_model_to_feature, R_model_to_world, xi, 3, 3);
  free_matrix2(R_model_to_world);

  // p(q2)
  int c = pcd->clusters[i];
  double p = bingham_mixture_pdf(q_model_to_feature, &olf->bmx[c]);

  // p(x2|q2)
  double x_mean[3];
  double *x_mean_ptr[1];
  x_mean_ptr[0] = x_mean;
  double **x_cov = new_matrix2(3,3);
  hll_sample(x_mean_ptr, &x_cov, q_model_to_feature_ptr, &olf->hll[c], 1);
  p *= mvnpdf(x_model_to_feature, x_mean, x_cov, 3);
  free_matrix2(x_cov);
  

  //dbug
  //printf("q_feature = (%f, %f, %f, %f)\n", q_feature[0], q_feature[1], q_feature[2], q_feature[3]);
  //printf("q_model_to_world = (%f, %f, %f, %f)\n", q_model_to_world[0], q_model_to_world[1], q_model_to_world[2], q_model_to_world[3]);

  //dbug
  //printf("x_model_to_feature = (%f, %f, %f)\n", x_model_to_feature[0], x_model_to_feature[1], x_model_to_feature[2]);
  //printf("q_model_to_feature = (%f, %f, %f, %f)\n", q_model_to_feature[0], q_model_to_feature[1], q_model_to_feature[2], q_model_to_feature[3]);

  return p;
}
*/

/*
 * samples n weighted poses (X,Q,W) using olf model "olf" and point cloud "pcd"
 *
olf_pose_samples_t *olf_pose_sample(olf_t *olf, pcd_t *pcd, int n)
{
  double epsilon = 1e-50;

  olf_pose_samples_t *poses = olf_pose_samples_new(n);

  double **X = poses->X;
  double **Q = poses->Q;
  double *W = poses->W;

  int num_validators = olf->num_validators;
  int npoints = pcd->num_points;

  double *q_feature, q_model_to_feature[4], q_feature_to_model[4];
  double x_mean[3], **x_cov = new_matrix2(3,3);
  double x_feature[3], x_model_to_feature[3], x_model_to_feature_rot[3];
  double **R = new_matrix2(3,3);
  int indices[num_validators];
  int close_points[npoints];

  // pointers
  double *q_model_to_feature_ptr[1], *x_mean_ptr[1];
  q_model_to_feature_ptr[0] = q_model_to_feature;
  x_mean_ptr[0] = x_mean;

  // proposal weights
  int i;
  int *cluster_indices[olf->num_clusters];
  int cluster_counts[olf->num_clusters];
  double proposal_weights[olf->num_clusters];
  if (olf->proposal_weights) {
    for (i = 0; i < olf->num_clusters; i++) {
      safe_malloc(cluster_indices[i], npoints, int);
      cluster_counts[i] = findeq(cluster_indices[i], pcd->clusters, i, npoints);
      proposal_weights[i] = cluster_counts[i] * olf->proposal_weights[i];
    }
    mult(proposal_weights, proposal_weights, 1.0/sum(proposal_weights, olf->num_clusters), olf->num_clusters);
  }

  // segment indices (for cluttered pcds)
  int **segment_indices = NULL;
  int *segment_cnts = NULL;
  if (olf->cluttered) {
    segment_indices = new_matrix2i(pcd->balls->num_segments, pcd->num_points);
    safe_calloc(segment_cnts, pcd->balls->num_segments, int);
    for (i = 0; i < pcd->num_points; i++) {
      int s = pcd->balls->segment_labels[i];
      segment_indices[s][ segment_cnts[s]++ ] = i;
    }
  }


  for (i = 0; i < n; i++) {
    // sample a proposal feature
    int j = 0;
    if (olf->proposal_weights) {
      int cluster = pmfrand(proposal_weights, olf->num_clusters);
      j = cluster_indices[cluster][ irand(cluster_counts[cluster]) ];
    }
    else if (olf->num_proposal_segments > 0) {
      //printf("break 1\n");
      //printf("olf->num_proposal_segments = %d\n", olf->num_proposal_segments);
      int s = olf->proposal_segments[ irand(olf->num_proposal_segments) ];
      //printf("s = %d\n", s);
      j = segment_indices[s][ irand(segment_cnts[s]) ];
      printf("proposal = %d\n", j);
    }
    else
      j = irand(npoints);

    q_feature = (frand() < .5 ? pcd->quaternions[0][j] : pcd->quaternions[1][j]);

    x_feature[0] = pcd->points[0][j];
    x_feature[1] = pcd->points[1][j];
    x_feature[2] = pcd->points[2][j];


    *** dbug
    double x_dbug[3] = {1000,0,0};    // world to model
    double q_dbug[4] = {0,1,0,0};  // world to model
    double q_feature_dbug[4];
    quaternion_mult(q_feature_dbug, q_feature, q_dbug);
    q_feature = q_feature_dbug;
    double **R_dbug = new_matrix2(3,3);
    quaternion_to_rotation_matrix(R_dbug, q_dbug);
    double x_feature_dbug[3];
    matrix_vec_mult(x_feature_dbug, R_dbug, x_feature, 3, 3);
    add(x_feature, x_feature_dbug, x_dbug, 3);
    free_matrix2(R_dbug);
    ***

    // sample model orientation
    int c = pcd->clusters[j];
    bingham_mixture_sample(q_model_to_feature_ptr, &olf->bmx[c], 1);
    quaternion_inverse(q_feature_to_model, q_model_to_feature);
    quaternion_mult(Q[i], q_feature_to_model, q_feature);

    // sample model position given orientation
    hll_sample(x_mean_ptr, &x_cov, q_model_to_feature_ptr, &olf->hll[c], 1);
    mvnrand(x_model_to_feature, x_mean, x_cov, 3);
    quaternion_to_rotation_matrix(R, Q[i]);
    matrix_vec_mult(x_model_to_feature_rot, R, x_model_to_feature, 3, 3);
    sub(X[i], x_feature, x_model_to_feature_rot, 3);

    // if cluttered, use smart validation (with segmentation/balls model)
    int k, valid = 1;
    if (olf->cluttered) {
      if (olf->num_proposal_segments > 0) { //dbug
	// sample validation points
	for (k = 0; k < num_validators; k++) {
	  int s = olf->proposal_segments[ irand(olf->num_proposal_segments) ];
	  indices[k] = segment_indices[s][ irand(segment_cnts[s]) ];
	  //printf("validation = %d\n", indices[k]);
	}
      }
      else {
	int num_close_points = pcd_intersect(close_points, pcd, olf->pcd, X[i], Q[i]);
	if (num_close_points == 0)
	  valid = 0;
	else {
	  printf("num_close_points = %d\n", num_close_points); //dbug
	  // sample validation points
	  for (k = 0; k < num_validators; k++)
	    indices[k] = close_points[ irand(num_close_points) ];
	}
      }
    }
    else {
      // sample validation points
      for (k = 0; k < num_validators; k++)
	indices[k] = irand(npoints);
      //randperm(indices, npoints, num_validators);
    }

    //dbug: test random walk
    //double sigma = 30;
    //int indices_walk[10*num_validators];
    //pcd_random_walk(indices_walk, pcd, j, 10*num_validators, sigma/10.0);
    //int k;
    //for (k = 0; k < num_validators; k++)
    //  indices[k] = indices_walk[10*k];

    ***dbug
    if (i == 0) {
      memcpy(X[i], x_dbug, 3*sizeof(double));
      memcpy(Q[i], q_dbug, 4*sizeof(double));
      indices[0] = 50;
      indices[1] = 100;
      indices[2] = 150;
      indices[3] = 200;
      indices[4] = 250;
      //printf("x = {%f, %f, %f}; q = {%f, %f, %f, %f};\n", X[i][0], X[i][1], X[i][2], Q[i][0], Q[i][1], Q[i][2], Q[i][3]);
      //printf("indices = {");
      //for (k = 0; k < num_validators; k++)
      //  printf("%d ", indices[k]);
      //printf("};\n");
    }
    ***

    // compute target density for the given pose
    if (!valid)
      W[i] = epsilon;
    else if (num_validators > 0)
      W[i] = olf_pose_pdf(X[i], Q[i], olf, pcd, indices, num_validators);
    else
      W[i] = 1.0;

    // dbug
    //if (i == 0) {
    //  printf("W[0] = %e\n", W[i]);
    //  exit(1);
    //}
  }

  // sort pose samples by weight
  int I[n];
  double W2[n];
  mult(W2, W, -1, n);
  sort_indices(W2, I, n);  // sort -W (i.e. descending W)

  for (i = 0; i < n; i++)
    W[i] = -W2[I[i]];
  reorder_rows(X, X, I, n, 3);
  reorder_rows(Q, Q, I, n, 4);

  // normalize W
  mult(W, W, 1.0/sum(W,n), n);

  free_matrix2(x_cov);
  free_matrix2(R);

  if (olf->cluttered) {
    free_matrix2i(segment_indices);
    free(segment_cnts);
  }

  return poses;
}
*/

/*
 * aggregate the weighted pose samples, (X,Q,W)
 *
olf_pose_samples_t *olf_aggregate_pose_samples(olf_pose_samples_t *poses, olf_t *olf)
{
  olf_pose_samples_t *agg_poses = olf_pose_samples_new(poses->n);

  double **R1 = new_matrix2(3,3);
  double **R2 = new_matrix2(3,3);
  double z1[3], z2[3], z[3];

  int i, j, cnt=0;
  for (i = 0; i < poses->n; i++) {
    
    if (olf->rot_symm) {
      quaternion_to_rotation_matrix(R1, poses->Q[i]);
      z1[0] = R1[0][2];
      z1[1] = R1[1][2];
      z1[2] = R1[2][2];
    }

    for (j = 0; j < cnt; j++) {
      double dx = dist(poses->X[i], agg_poses->X[j], 3);
      double dq = 0.0;
      if (olf->rot_symm) {
	quaternion_to_rotation_matrix(R2, agg_poses->Q[j]);
	z2[0] = R2[0][2];
	z2[1] = R2[1][2];
	z2[2] = R2[2][2];
	dq = acos(dot(z1, z2, 3));
      }
      else
	dq = acos(fabs(dot(poses->Q[i], agg_poses->Q[j], 4)));
      
      //fprintf(stderr, "dx = %3.0f, dq = %3.0f\n", dx, dq*(180.0/M_PI));

      //if (a*a*dx*dx + b*b*dq*dq < 1.0) {  // add pose i to cluster j
      if (dx < olf->pose_agg_x && dq < olf->pose_agg_q) {  // add pose i to cluster j
	double wtot = poses->W[i] + agg_poses->W[j];
	double w = poses->W[i] / wtot;
	wavg(agg_poses->X[j], poses->X[i], agg_poses->X[j], w, 3);
	if (olf->rot_symm) {
	  wavg(z, z1, z2, w, 3);
	  normalize(z, z, 3);
	  if (1 - z[2] < .00000001) {  // close to identity rotation
	    agg_poses->Q[j][0] = 1;
	    agg_poses->Q[j][1] = 0;
	    agg_poses->Q[j][2] = 0;
	    agg_poses->Q[j][3] = 0;
	  }
	  else {
	    double a = 1.0 / sqrt(1 - z[2]*z[2]);
	    double c = sqrt((1 + z[2])/2.0);
	    double s = sqrt((1 - z[2])/2.0);
	    agg_poses->Q[j][0] = c;
	    agg_poses->Q[j][1] = -s*a*z[1];
	    agg_poses->Q[j][2] = s*a*z[0];
	    agg_poses->Q[j][3] = 0;
	  }
	  
	}
	else {
	  wavg(agg_poses->Q[j], poses->Q[i], agg_poses->Q[j], w, 4);
	  normalize(agg_poses->Q[j], agg_poses->Q[j], 4);
	}
	agg_poses->W[j] = wtot;
	break;
      }
    }
    if (j == cnt) {  // add a new cluster
      memcpy(agg_poses->X[cnt], poses->X[i], 3*sizeof(double));
      memcpy(agg_poses->Q[cnt], poses->Q[i], 4*sizeof(double));
      agg_poses->W[cnt] = poses->W[i];
      cnt++;
    }
  }

  free_matrix2(R1);
  free_matrix2(R2);

  // sort pose samples by weight
  int n = cnt;
  int I[n];
  double W2[n];
  mult(W2, agg_poses->W, -1, n);
  sort_indices(W2, I, n);  // sort -W (i.e. descending W)

  for (i = 0; i < n; i++)
    agg_poses->W[i] = -W2[I[i]];
  reorder_rows(agg_poses->X, agg_poses->X, I, n, 3);
  reorder_rows(agg_poses->Q, agg_poses->Q, I, n, 4);

  agg_poses->n = n;

  return agg_poses;
}
*/

/*
 * create a new olf_pose_samples_t
 *
olf_pose_samples_t *olf_pose_samples_new(int n)
{
  olf_pose_samples_t *poses;
  safe_calloc(poses, 1, olf_pose_samples_t);
  poses->X = new_matrix2(n,3);
  poses->Q = new_matrix2(n,4);
  safe_calloc(poses->W, n, double);
  poses->n = n;

  return poses;
}
*/

/*
 * free pose samples
 *
void olf_pose_samples_free(olf_pose_samples_t *poses)
{
  if (poses->X)
    free_matrix2(poses->X);
  if (poses->Q)
    free_matrix2(poses->Q);
  if (poses->W)
    free(poses->W);
  free(poses);
}
*/


