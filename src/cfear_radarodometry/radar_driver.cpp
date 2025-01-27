#include "cfear_radarodometry/radar_driver.h"

namespace CFEAR_Radarodometry {



filtertype Str2filter(const std::string& str){
  if (str=="CA-CFAR")
    return filtertype::CACFAR;
  else
    return filtertype::kstrong;

}

std::string Filter2str(const filtertype& filter){
  switch (filter){
  case filtertype::CACFAR: return "CA-CFAR";
  case kstrong: return "kstrong";
  }
  return "kstrong";
}

radarDriver::radarDriver(const Parameters& pars, bool disable_callback):par(pars),nh_("~"), it(nh_) {


  min_distance_sqrd = par.min_distance*par.min_distance;
  FilteredPublisher = nh_.advertise<pcl::PointCloud<pcl::PointXYZI>>(par.topic_filtered, 1000);
  if(!disable_callback){
    if(par.dataset=="oxford")
      sub = nh_.subscribe<sensor_msgs::Image>("/Navtech/Polar", 1000, &radarDriver::CallbackOxford, this);
    else
      sub = nh_.subscribe<sensor_msgs::Image>("/Navtech/Polar", 1000, &radarDriver::Callback, this);
  }
}

void radarDriver::InitAngles(){
  sin_values.resize(par.azimuths);
  cos_values.resize(par.azimuths);
  for (int i=0;i<par.azimuths;i++){
    float theta=(float)(i+1)*2*M_PI/par.azimuths;
    sin_values[i]=std::sin(theta);
    cos_values[i]=std::cos(theta);
  }
}

void radarDriver::Callback(const sensor_msgs::ImageConstPtr &radar_image_polar){
    if(radar_image_polar==NULL){
        cerr<<"Radar image NULL"<<endl;
        exit(0);
    }
  ros::Time t0 = ros::Time::now();
  cv_bridge::CvImagePtr cv_polar_image;
  cv_polar_image = cv_bridge::toCvCopy(radar_image_polar, sensor_msgs::image_encodings::MONO8);
  cv_polar_image->header.stamp = radar_image_polar->header.stamp;
  cv::resize(cv_polar_image->image,cv_polar_image->image,cv::Size(), 1, 1, cv::INTER_NEAREST);
  uint16_t bins = cv_polar_image->image.rows;
  rotate(cv_polar_image->image, cv_polar_image->image, cv::ROTATE_90_COUNTERCLOCKWISE);

  cloud_filtered = pcl::PointCloud<pcl::PointXYZI>::Ptr(new pcl::PointCloud<pcl::PointXYZI>());
  CFEAR_Radarodometry::k_strongest_filter(cv_polar_image, cloud_filtered, par.k_strongest, par.z_min, par.range_res, par.min_distance);
  cloud_filtered->header.frame_id = par.radar_frameid;
  ros::Time tstamp = radar_image_polar->header.stamp.toSec() < 0.001 ? ros::Time::now() : radar_image_polar->header.stamp;; //rosparam set use_sim_time true
  pcl_conversions::toPCL(tstamp, cloud_filtered->header.stamp);
  FilteredPublisher.publish(cloud_filtered);
  ros::Time t2 = ros::Time::now();
  CFEAR_Radarodometry::timing.Document("Filtering",CFEAR_Radarodometry::ToMs(t2-t0));


}

/*Assumptions:
       *
       * TYPE_8UC1
       * Rows azimuth
       * Cols Range, from left (rmin) to right (rmax)
       *
       * */
void radarDriver::CallbackOxford(const sensor_msgs::ImageConstPtr &radar_image_polar)
{
    if(radar_image_polar==NULL){
        cerr<<"Radar image NULL"<<endl;
        exit(0);
    }
  ros::Time t0 = ros::Time::now();
  cv_bridge::CvImagePtr cv_polar_image;
  cv_polar_image = cv_bridge::toCvCopy(radar_image_polar, sensor_msgs::image_encodings::TYPE_8UC1);
  //ros::Time t2 = ros::Time::now();
  //CFEAR_Radarodometry::timing.Document("Filtering",CFEAR_Radarodometry::ToMs(t2-t0));
  cloud_filtered = pcl::PointCloud<pcl::PointXYZI>::Ptr(new pcl::PointCloud<pcl::PointXYZI>());
  // Implementation (1)
  //CFEAR_Radarodometry::kstrongLookup filter(par.k_strongest, par.z_min,par.range_res,par.min_distance);
  //filter.getFilteredPointCloud(cv_polar_image, cloud_filtered);

  //Implementation (2)
  //CFEAR_Radarodometry::k_strongest_filter(cv_polar_image, cloud_filtered, par.k_strongest, par.z_min, par.range_res, par.min_distance);

  //Implementation (3)
  //StructuredKStrongest filt(cv_polar_image, par.z_min, par.k_strongest, par.min_distance, par.range_res);
  //filt.getPeaksFilteredPointCloud(cloud_filtered, false);
  if(par.filter_type_ == filtertype::CACFAR) {
    cout<<"window: "<<par.window_size<<", par:false alarm:"<<par.false_alarm_rate<<", par:nb_guard:"<<par.nb_guard_cells<<endl;
    AzimuthCACFAR filter(par.window_size, par.false_alarm_rate, par.nb_guard_cells, par.range_res, par.z_min , par.min_distance, 400.0);
    filter.getFilteredPointCloud(cv_polar_image,cloud_filtered);
  }
  else{
    StructuredKStrongest filt(cv_polar_image, par.z_min, par.k_strongest, par.min_distance, par.range_res);
    filt.getPeaksFilteredPointCloud(cloud_filtered, false);
 // For visualization of k=1,12,40. Fig.3 in paper.
    static ros::NodeHandle nh("~");


    /* For figures presnted in CFEAR Radarodometry - Lidar-levle localization with radar
    static ros::Publisher pub1 = nh.advertise<pcl::PointCloud<pcl::PointXYZI>>("kstrong1", 100);
    static ros::Publisher pub12 = nh.advertise<pcl::PointCloud<pcl::PointXYZI>>("kstrong12", 100);
    static ros::Publisher pub40 = nh.advertise<pcl::PointCloud<pcl::PointXYZI>>("kstrong40", 100);

    pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_filtered1 = pcl::PointCloud<pcl::PointXYZI>::Ptr(new pcl::PointCloud<pcl::PointXYZI>());
    StructuredKStrongest filt1(cv_polar_image, par.z_min, 1, par.min_distance, par.range_res);
    filt1.getPeaksFilteredPointCloud(cloud_filtered1, false);
    for(auto && p : cloud_filtered1->points)
      p.z = 2;

    pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_filtered12 = pcl::PointCloud<pcl::PointXYZI>::Ptr(new pcl::PointCloud<pcl::PointXYZI>());
    StructuredKStrongest filt12(cv_polar_image, par.z_min, 12, par.min_distance, par.range_res);
    filt12.getPeaksFilteredPointCloud(cloud_filtered12, false);
    for(auto && p : cloud_filtered12->points)
      p.z = 1;

    pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_filtered40 = pcl::PointCloud<pcl::PointXYZI>::Ptr(new pcl::PointCloud<pcl::PointXYZI>());
    StructuredKStrongest filt40(cv_polar_image, par.z_min, 40, par.min_distance, par.range_res);
    filt40.getPeaksFilteredPointCloud(cloud_filtered40, false);

    cloud_filtered1->header.frame_id = cloud_filtered12->header.frame_id = cloud_filtered40->header.frame_id = "world";
    pub1.publish(*cloud_filtered1);
    pub12.publish(*cloud_filtered12);
    pub40.publish(*cloud_filtered40);
    Eigen::Affine3d T = Eigen::Affine3d::Identity();
    CFEAR_Radarodometry::MapNormalPtr Pcurrent = CFEAR_Radarodometry::MapNormalPtr(new MapPointNormal(cloud_filtered12, 3.0, Eigen::Vector2d(0,0), true, false));
    MapPointNormal::PublishMap("/current_normals_k12",Pcurrent,T, "world", 0);
    CFEAR_Radarodometry::MapNormalPtr Pcurrent2 = CFEAR_Radarodometry::MapNormalPtr(new MapPointNormal(cloud_filtered40, 3.0, Eigen::Vector2d(0,0), true, false));
    MapPointNormal::PublishMap("/current_normals_k40", Pcurrent2, T, "world", 0);
    char c = std::getchar();
    */

  }




  ros::Time tstamp = radar_image_polar->header.stamp.toSec() < 0.001 ? ros::Time::now() : radar_image_polar->header.stamp;; //rosparam set use_sim_time true
  pcl_conversions::toPCL(tstamp, cloud_filtered->header.stamp);
  cloud_filtered->header.frame_id = par.radar_frameid;
  std::cout << std::to_string(tstamp.toSec()) << std::endl;
  FilteredPublisher.publish(cloud_filtered);
  ros::Time t3 = ros::Time::now();
  CFEAR_Radarodometry::timing.Document("time-Filtering",CFEAR_Radarodometry::ToMs(t3-t0));
}

void radarDriver::CallbackOffline(const sensor_msgs::ImageConstPtr& radar_image_polar,  pcl::PointCloud<pcl::PointXYZI>::Ptr& cloud){

  if(par.dataset=="oxford")
    CallbackOxford(radar_image_polar);
  else
    Callback(radar_image_polar);

  *cloud = *cloud_filtered;
  cloud->header = cloud_filtered->header;

}

}
