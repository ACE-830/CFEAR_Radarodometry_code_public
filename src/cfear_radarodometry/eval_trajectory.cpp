#include "cfear_radarodometry/eval_trajectory.h"

namespace CFEAR_Radarodometry {


EvalTrajectory::EvalTrajectory(const EvalTrajectory::Parameters& pars, bool disable_callback) :par(pars), nh_("~"),downsampled(new pcl::PointCloud<pcl::PointXYZI>()){

  if(!disable_callback){
    assert(!par.odom_gt_topic.empty() && !par.odom_est_topic.empty());
    if(par.synced_callback){
      pose_sub_gt  = new message_filters::Subscriber<nav_msgs::Odometry>(nh_, par.odom_gt_topic, 100);
      pose_sub_est  = new message_filters::Subscriber<nav_msgs::Odometry>(nh_, par.odom_est_topic, 100);
      sync = new Synchronizer<double_odom>(double_odom(100), *pose_sub_gt, *pose_sub_est);
      sync->registerCallback(boost::bind(&EvalTrajectory::CallbackSynchronized,this, _1, _2));
    }
    else{
      sub_est = nh_.subscribe(par.odom_est_topic, 1000, &EvalTrajectory::CallbackEst, this);
      sub_gt = nh_.subscribe(par.odom_gt_topic, 1000, &EvalTrajectory::CallbackGT, this);
    }
  }
  pub_est = nh_.advertise<nav_msgs::Path>("path_est", 10);
  pub_gt = nh_.advertise<nav_msgs::Path>("path_gt", 10);
  pub_cloud = nh_.advertise<pcl::PointCloud<pcl::PointXYZI>>("map_cloud", 10);

}


void EvalTrajectory::CallbackSynchronized(const nav_msgs::Odometry::ConstPtr& msg_est, const nav_msgs::Odometry::ConstPtr& msg_gt){
  cout<<"callback"<<endl;
  Eigen::Affine3d T;
  tf::poseMsgToEigen(msg_est->pose.pose, T);
  gt_vek.push_back(std::make_pair(T, msg_est->header.stamp));

  tf::poseMsgToEigen(msg_gt->pose.pose, T);
  est_vek.push_back(std::make_pair(T, msg_gt->header.stamp));
}
void EvalTrajectory::CallbackEigen(const poseStamped& Test, const poseStamped& Tgt){
  gt_vek.push_back(Tgt);
  est_vek.push_back(Test);
}
void EvalTrajectory::CallbackGTEigen(const poseStamped& Tgt){
  gt_vek.push_back(Tgt);
}

void EvalTrajectory::CallbackESTEigen(const poseStamped& Test){
  est_vek.push_back(Test);
}
void EvalTrajectory::CallbackESTEigen(const poseStamped& Test, const pcl::PointCloud<pcl::PointXYZI>& cld){
  clouds.push_back(cld);
  CallbackESTEigen(Test);
}

void EvalTrajectory::CallbackGT(const nav_msgs::Odometry::ConstPtr &msg){
  Eigen::Affine3d T;
  ros::Time t = msg->header.stamp;
  tf::poseMsgToEigen(msg->pose.pose, T);
  gt_vek.push_back(std::make_pair(T, t));
}

void EvalTrajectory::CallbackEst(const nav_msgs::Odometry::ConstPtr &msg){
  Eigen::Affine3d T;
  tf::poseMsgToEigen(msg->pose.pose, T);
  ros::Time t = msg->header.stamp;
  est_vek.push_back(std::make_pair(T, t));
}

std::string EvalTrajectory::DatasetToSequence(const std::string& dataset){
  if(dataset=="2019-01-10-11-46-21-radar-oxford-10k")
    return "00.txt";
  else if(dataset=="2019-01-10-12-32-52-radar-oxford-10k")
    return "01.txt";
  else if(dataset=="2019-01-10-14-02-34-radar-oxford-10k")
    return "02.txt";
  else if(dataset=="2019-01-10-14-36-48-radar-oxford-10k-partial")
    return "03.txt";
  else if(dataset=="2019-01-10-14-50-05-radar-oxford-10k")
    return "04.txt";
  else if(dataset=="2019-01-10-15-19-41-radar-oxford-10k")
    return "05.txt";
  else if(dataset=="2019-01-11-12-26-55-radar-oxford-10k")
    return "06.txt";
  else if(dataset=="2019-01-11-13-24-51-radar-oxford-10k")
    return "07.txt";
  else if(dataset=="2019-01-11-14-02-26-radar-oxford-10k")
    return "08.txt";
  else if(dataset=="2019-01-11-14-37-14-radar-oxford-10k")
    return "09.txt";
  else if(dataset=="2019-01-14-12-05-52-radar-oxford-10k")
    return "10.txt";
  else if(dataset=="2019-01-14-12-41-28-radar-oxford-10k")
    return "11.txt";
  else if(dataset=="2019-01-14-13-38-21-radar-oxford-10k")
    return "12.txt";
  else if(dataset=="2019-01-14-14-15-12-radar-oxford-10k")
    return "13.txt";
  else if(dataset=="2019-01-14-14-48-55-radar-oxford-10k")
    return "14.txt";
  else if(dataset=="2019-01-15-12-01-32-radar-oxford-10k")
    return "15.txt";
  else if(dataset=="2019-01-15-12-52-32-radar-oxford-10k-partial")
    return "16.txt";
  else if(dataset=="2019-01-15-13-06-37-radar-oxford-10k")
    return "17.txt";
  else if(dataset=="2019-01-15-13-53-14-radar-oxford-10k")
    return "18.txt";
  else if(dataset=="2019-01-15-14-24-38-radar-oxford-10k")
    return "19.txt";
  else if(dataset=="2019-01-16-11-53-11-radar-oxford-10k")
    return "20.txt";
  else if(dataset=="2019-01-16-13-09-37-radar-oxford-10k")
    return "21.txt";
  else if(dataset=="2019-01-16-13-42-28-radar-oxford-10k")
    return "22.txt";
  else if(dataset=="2019-01-16-14-15-33-radar-oxford-10k")
    return "23.txt";
  else if(dataset=="2019-01-17-11-46-31-radar-oxford-10k")
    return "24.txt";
  else if(dataset=="2019-01-17-12-48-25-radar-oxford-10k")
    return "25.txt";
  else if(dataset=="2019-01-17-13-26-39-radar-oxford-10k")
    return "26.txt";
  else if(dataset=="2019-01-17-14-03-00-radar-oxford-10k")
    return "27.txt";
  else if(dataset=="2019-01-18-12-42-34-radar-oxford-10k")
    return "28.txt";
  else if(dataset=="2019-01-18-14-14-42-radar-oxford-10k")
    return "29.txt";
  else if(dataset=="2019-01-18-14-46-59-radar-oxford-10k")
    return "30.txt";
  else if(dataset=="2019-01-18-15-20-12-radar-oxford-10k")
    return "31.txt";
  else if(dataset=="2019-01-18-14-46-59-radar-oxford-10k")
    return "32.txt";
  else return "01.txt";

}
void EvalTrajectory::RemoveExtras(){
  //cerr<<"Before remove. est_vek.size()="<<est_vek.size()<<", gt_vek.size()="<<gt_vek.size()<<"\n";

  while( !est_vek.empty() || !gt_vek.empty() ){

    if( fabs( ros::Duration(est_vek.front().second - gt_vek.front().second).toSec() ) > 0.0001  ){
      if(est_vek.front().second < gt_vek.front().second){
        est_vek.erase(est_vek.begin());
        if(!clouds.empty())
          clouds.erase(clouds.begin());
      }
      else
        gt_vek.erase(gt_vek.begin());
    }
    else if( fabs( ros::Duration(est_vek.back().second - gt_vek.back().second).toSec() ) > 0.0001  ){
      if(est_vek.back().second > gt_vek.back().second){
        if(!clouds.empty())
          clouds.pop_back();
        est_vek.pop_back();
      }
      else
        gt_vek.pop_back();
    }
    else {
      break;
    }
  }
  return;
}
void EvalTrajectory::Write(const std::string& path, const poseStampedVector& v){
  std::ofstream evalfile;
  cout<<"Saving: "<<v.size()<<" poses to file: "<<path<<endl;
  evalfile.open(path);
  for(size_t i=0;i<v.size();i++){
    Eigen::MatrixXd m(v[i].first.matrix());
    evalfile<< std::fixed << std::showpoint;
    assert(m.rows()== 4 && m.cols()==4);

    evalfile<< m(0,0) <<" "<< m(0,1) <<" "<< m(0,2) <<" "<< m(0,3) <<" "<<
               m(1,0) <<" "<< m(1,1) <<" "<< m(1,2) <<" "<< m(1,3) <<" "<<
               m(2,0) <<" "<< m(2,1) <<" "<< m(2,2) <<" "<< m(2,3) <<std::endl;
  }
  evalfile.close();
  return;
}
void Plot(){

}
void EvalTrajectory::PublishTrajectory(poseStampedVector& vek, ros::Publisher& pub){
  nav_msgs::Path path;
  path.header.frame_id="world";
  path.header.stamp = ros::Time::now();

  std::vector<tf::StampedTransform> trans_vek;
  for (int i=0;i<vek.size();i++) {
    Eigen::Affine3d T = vek[i].first;
    geometry_msgs::PoseStamped Tstamped;
    tf::poseEigenToMsg(T,Tstamped.pose);
    path.poses.push_back(Tstamped);
  }
  pub.publish(path);
}
void EvalTrajectory::PrintStatus(){
  if(gt_vek.size()!=est_vek.size()){
    //    std::cerr<<"SIZE ERROR. est_vek.size()="<<est_vek.size()<<", gt_vek.size()="<<gt_vek.size() <<std::endl;
  }
  cout<<"Est first: "<<est_vek.front().first.translation().transpose()<<" - time: "<<est_vek.front().second<<endl;
  cout<<"GT: first: "<<gt_vek.front().first.translation().transpose()<<" - time: "<<gt_vek.front().second<<endl;

  cout<<"Est Last: "<<est_vek.back().first.translation().transpose()<<" - time: "<<est_vek.back().second<<endl;
  cout<<"GT: Last: "<<gt_vek.back().first.translation().transpose()<<" - time: "<<gt_vek.back().second<<endl;
  return;
}
void EvalTrajectory::SavePCD(const std::string& folder){
  for (size_t i=0;i<clouds.size();i++) {
    pcl::PointCloud<pcl::PointXYZI> cld_transformed;
    pcl::transformPointCloud(clouds[i], cld_transformed, est_vek[i].first);
    pcl::io::savePCDFileBinary(folder+"cloud_"+std::to_string(i)+std::string(".pcd"), cld_transformed);
  }
}

void EvalTrajectory::Save(){
  cout << "Saving, outpout: " << par.est_output_dir << std::endl;

  if(est_vek.empty() && gt_vek.empty()){
    cout<<"Nothing to evaluate"<<endl;
    cerr<<"array size error. est_vek.size()="<<est_vek.size()<<", gt_vek.size()="<<gt_vek.size()<<endl;
    exit(0);
  }
  else if(gt_vek.empty()){
    cout<<"No ground truth? No problem! without gps the need of radar based localization is even larger"<<endl;
    boost::filesystem::create_directories(par.est_output_dir);
    std::string est_path = par.est_output_dir+DatasetToSequence(par.sequence);     cout<<"Saving estimate poses only, no ground truth, total: "<<est_vek.size()<<" poses"<<endl;
    cout<<"To path: "<<est_path<<endl;
    if(par.save_pcd)
      SavePCD(par.est_output_dir);
    Write(est_path,  est_vek);
    cout<<"Trajectoy saved"<<endl;
    return;
    
  }else if(est_vek.size()!=gt_vek.size()){
    //PrintStatus();
    //RemoveExtras();
    //PrintStatus();
    One2OneCorrespondance();
  }
  else if(par.method == "floam"){ // only needed for lidar and mulran to interpolate timetamps
    One2OneCorrespondance();
  }

  //AlignTrajectories();
  /*ros::Rate r(1);
  //while(ros::ok()){
    //PublishTrajectory(est_vek, pub_est);
    PublishTrajectory(gt_vek, pub_gt);
    downsampled->header.frame_id = std::string("world");
    ros::Time t = ros::Time::now();
    pcl_conversions::toPCL(t,downsampled->header.stamp);
    //pub_cloud.publish(*downsampled);
    r.sleep();
  }*/
  cout << "create: " << par.gt_output_dir << std::endl;
  boost::filesystem::create_directories(par.gt_output_dir);
  cout << "create: " << par.est_output_dir << std::endl;
  boost::filesystem::create_directories(par.est_output_dir);
  std::string gt_path  = par.gt_output_dir +DatasetToSequence(par.sequence);
  std::string est_path = par.est_output_dir+DatasetToSequence(par.sequence);
  cout<<"Saving est_vek.size()="<<est_vek.size()<<", gt_vek.size()="<<gt_vek.size()<<endl;
  cout<<"To path: \n\" "<<gt_path<<"\""<<"\n\""<<est_path<<"\""<<endl;
  if(par.save_pcd)
    SavePCD(par.est_output_dir);
  Write(gt_path, gt_vek);
  Write(est_path,  est_vek);
  cout<<"Trajectories saved"<<endl;

  return;
}
void EvalTrajectory::AlignTrajectories(){
  cout<<"align"<<endl;
  if(est_vek.size()!=gt_vek.size())
    return;
  size_t n = est_vek.size();
  Eigen::MatrixXd traj_est(n,3), traj_gt(n,3);
  for(size_t i=0 ; i<n ; i++){
    traj_est.block<1,3>(i,0) = est_vek[i].first.translation().transpose();
    traj_gt.block<1,3>(i,0) = gt_vek[i].first.translation().transpose();
  }
  Eigen::Matrix4d transform = best_fit_transform(traj_gt,traj_est);
  Eigen::Affine3d T(transform);
  double sum = 0;

  //pcl::PointCloud<pcl::PointXYZI>::Ptr tmp_merged(new pcl::PointCloud<pcl::PointXYZI>());
  //pcl::PointCloud<pcl::PointXYZI>::Ptr tmp_statistics_filtered(new pcl::PointCloud<pcl::PointXYZI>());
  for(size_t i=0 ; i<n ; i++){
    est_vek[i].first = T.inverse()*est_vek[i].first;
    double d = (est_vek[i].first.translation() - gt_vek[i].first.translation()).norm();
    sum +=d*d;
  }

  /*cout<<"merged size: "<<tmp_merged->size();



  //pcl::VoxelGrid<pcl::PointXYZ> sor;
  pcl::RandomSample<pcl::PointXYZI> sor;
  sor.setInputCloud (tmp_merged);
  sor.setSample(5000000);
  //sor.setLeafSize (0.04f, 0.04f, 0.04f);
  sor.filter (*downsampled);
*/
  const double ATE = std::sqrt(sum/n);
  cout<<"ATE: "<<ATE<<endl;
  cout<<"result: "<<transform<<endl;

}
Eigen::Matrix4d EvalTrajectory::best_fit_transform(const Eigen::MatrixXd &A, const Eigen::MatrixXd &B){
  /*
    Notice:
    1/ JacobiSVD return U,S,V, S as a vector, "use U*S*Vt" to get original Matrix;
    2/ matrix type 'MatrixXd' or 'MatrixXf' matters.
    */
  Eigen::Matrix4d T = Eigen::MatrixXd::Identity(4,4);
  Eigen::Vector3d centroid_A(0,0,0);
  Eigen::Vector3d centroid_B(0,0,0);
  Eigen::MatrixXd AA = A;
  Eigen::MatrixXd BB = B;
  int row = A.rows();

  for(int i=0; i<row; i++){
    centroid_A += A.block<1,3>(i,0).transpose();
    centroid_B += B.block<1,3>(i,0).transpose();
  }
  centroid_A /= row;
  centroid_B /= row;
  for(int i=0; i<row; i++){
    AA.block<1,3>(i,0) = A.block<1,3>(i,0) - centroid_A.transpose();
    BB.block<1,3>(i,0) = B.block<1,3>(i,0) - centroid_B.transpose();
  }

  Eigen::MatrixXd H = AA.transpose()*BB;
  Eigen::MatrixXd U;
  Eigen::VectorXd S;
  Eigen::MatrixXd V;
  Eigen::MatrixXd Vt;
  Eigen::Matrix3d R;
  Eigen::Vector3d t;

  Eigen::JacobiSVD<Eigen::MatrixXd> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);
  U = svd.matrixU();
  S = svd.singularValues();
  V = svd.matrixV();
  Vt = V.transpose();

  R = Vt.transpose()*U.transpose();

  if (R.determinant() < 0 ){
    Vt.block<1,3>(2,0) *= -1;
    R = Vt.transpose()*U.transpose();
  }

  t = centroid_B - R*centroid_A;

  T.block<3,3>(0,0) = R;
  T.block<3,1>(0,3) = t;
  return T;

}
void EvalTrajectory::One2OneCorrespondance(){
  cout<<"force one to one. est: "<<est_vek.size()<<", gt: "<<gt_vek.size()<<endl;
  poseStampedVector revised_est, revised_gt;
  poseStampedVector::iterator init_guess = gt_vek.begin();
  for(size_t i=0 ; i<est_vek.size() ; i++){
    poseStamped interp_corr;
    if(Interpolate(est_vek[i].second, interp_corr, init_guess)){
      revised_est.push_back(est_vek[i]);
      revised_gt.push_back(interp_corr);
    }
    else
      init_guess = gt_vek.begin();//reset guess
  }
  est_vek.clear();
  gt_vek.clear();
  est_vek = revised_est;
  gt_vek = revised_gt;
  //cout<<"force one to one. est: "<<est_vek.size()<<", gt: "<<gt_vek.size()<<endl;
  //for(int i=0;i<est_vek.size() && i<gt_vek.size();i+=1){
  //  cout<<"est: "<<est_vek[i].first.translation().transpose()<<" gt: "<<gt_vek[i].first.translation().transpose()<<endl;
  //}
}
bool EvalTrajectory::SearchByTime(const ros::Time& t, poseStampedVector::iterator& itr){
  const double tsearch = t.toSec();
  if(gt_vek.size() <=2){
    itr = gt_vek.begin();
    return false;
  }
  poseStampedVector::iterator last = std::prev(gt_vek.end());

  for(auto &&it = itr ; itr != last; it++){
    double tprev = it->second.toSec();
    double tnext = std::next(it)->second.toSec();
    if(tprev <= tsearch && tsearch <= tnext ) {
      itr = it;
      //cout<<"min:"<<it->second<<" search: "<<t<<"max: "<<std::next(it)->second<<endl;
      return true;
    }
  }
  return false;
}
bool EvalTrajectory::Interpolate(const ros::Time& t, poseStamped& Tinterpolated, poseStampedVector::iterator& itr){
  //cout<<"search: "<<t<<endl;
  bool found = SearchByTime(t,itr);
  if(found){
    poseStamped Tbefore = *itr;
    poseStamped Tafter = *std::next(itr);

    Tinterpolated.first = pose_interp(t.toSec(), Tbefore.second.toSec(), Tafter.second.toSec(),
                                      Tbefore.first, Tafter.first);
    Tinterpolated.second = t;
    return true;
  }
  else return false;

}
Eigen::Affine3d EvalTrajectory::pose_interp(double t, double t1, double t2, Eigen::Affine3d const& aff1, Eigen::Affine3d const& aff2) {
  // assume here t1 <= t <= t2

  double alpha = 0.0;
  if (t2 != t1)
    alpha = (t - t1) / (t2 - t1);

  Eigen::Quaternion<double> rot1(aff1.linear());
  Eigen::Quaternion<double> rot2(aff2.linear());

  Eigen::Vector3d trans1 = aff1.translation();
  Eigen::Vector3d trans2 = aff2.translation();
  //cout<<"slerp: alpha:"<<alpha<<trans1.transpose()<<" - "<<trans2.transpose()<<endl;
  Eigen::Affine3d result;
  result.translation() = (1.0 - alpha) * trans1 + alpha * trans2;
  Eigen::Matrix3d R = rot1.slerp(alpha, rot2).toRotationMatrix();
  /*double yaw = R.eulerAngles(0,1,2)(2);


  Eigen::AngleAxisd rollAngle(0, Eigen::Vector3d::UnitX());
  Eigen::AngleAxisd pitchAngle(0, Eigen::Vector3d::UnitY());
  Eigen::AngleAxisd yawAngle(yaw, Eigen::Vector3d::UnitZ());
*/

  //Eigen::Quaternion<double> q = rollAngle * yawAngle * pitchAngle;
  //result.linear()         = R;//q.matrix();;
  result.linear() = R;
  result.translation()(2) = 0;

  return result;
}


}
