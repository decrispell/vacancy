/*
 * Copyright (C) 2019, unclearness
 * All rights reserved.
 */

#pragma once

#include "vacancy/camera.h"
#include "vacancy/common.h"
#include "vacancy/image.h"
#include "vacancy/mesh.h"

namespace vacancy {

// Voxel update type
enum class VoxelUpdate {
  kMax = 0,             // take max
  kWeightedAverage = 1  // Weighted Average like KinectFusion. truncation is
                        // necessary to get good result
};

struct VoxelUpdateOption {
  VoxelUpdate voxel_update{VoxelUpdate::kMax};
  int voxel_max_update_num{
      255};  // After updating voxel_max_update_num, no sdf update
  float voxel_update_weight{1.0f};  // only valid if kWeightedAverage is set
  bool use_truncation{true};
  float truncation_band{0.1f};  // only positive value is valid
};

struct VoxelCarverOption {
  Eigen::Vector3f bb_max;
  Eigen::Vector3f bb_min;
  float resolution{0.001f};
  bool sdf_minmax_normalize{true};
  std::string debug_dir{""};
  VoxelUpdateOption update_option;
};

struct Voxel {
  Eigen::Vector3i index{-1, -1, -1};      // voxel index
  int id{-1};
  Eigen::Vector3f pos{0.0f, 0.0f, 0.0f};  // center of voxel
  float sdf{0.0f};  // Signed Distance Function (SDF) value
  int update_num{0};
  bool outside{false};
  bool on_surface{false};
  Voxel();
  ~Voxel();
};

class VoxelGrid {
  std::vector<Voxel> voxels_;
  Eigen::Vector3f bb_max_;
  Eigen::Vector3f bb_min_;
  float resolution_{-1.0f};
  Eigen::Vector3i voxel_num_{0, 0, 0};
  int xy_slice_num_{0};

 public:
  VoxelGrid();
  ~VoxelGrid();
  bool Init(const Eigen::Vector3f& bb_max, const Eigen::Vector3f& bb_min,
            float resolution);
  const Eigen::Vector3i& voxel_num() const;
  const Voxel& get(int x, int y, int z) const;
  Voxel* get_ptr(int x, int y, int z);
  float resolution() const;
  void ResetOnSurface();
  bool initialized() const;
};

class VoxelCarver {
  VoxelCarverOption option_;
  std::unique_ptr<VoxelGrid> voxel_grid_;

  void UpdateOnSurface();
  void UpdateOnSurfaceWithPseudo();

 public:
  VoxelCarver();
  ~VoxelCarver();
  VoxelCarver(VoxelCarverOption option);
  void set_option(VoxelCarverOption option);
  bool Init();
  bool Carve(const Camera& camera, const Image1b& silhouette, Image1f* sdf);
  bool Carve(const Camera& camera, const Image1b& silhouette);
  bool Carve(const std::vector<Camera>& cameras,
             const std::vector<Image1b>& silhouettes);
  void ExtractVoxel(Mesh* mesh, bool inside_empty = true,
                    bool with_pseudo_surface = false);
  void ExtractIsoSurface(Mesh* mesh, double iso_level = 0.0);
};

void DistanceTransformL1(const Image1b& mask, Image1f* dist);
void MakeSignedDistanceField(const Image1b& mask, Image1f* dist,
                             bool minmax_normalize, bool use_truncation,
                             float truncation_band);
void SignedDistance2Color(const Image1f& sdf, Image3b* vis_sdf,
                          float min_negative_d, float max_positive_d);

}  // namespace vacancy
