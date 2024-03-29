#ifndef CAFFE_FIX_TPOOLING_LAYER_HPP
#define CAFFE_FIX_TPOOLING_LAYER_HPP

#include <vector>

#include "caffe/blob.hpp"
#include "caffe/layer.hpp"
#include "caffe/proto/caffe.pb.h"

namespace caffe {

/**
 * @brief Pools the input Series of images by taking the average. Output a fix length of images.
 */
template <typename Dtype>
class FixTPoolingLayer : public Layer<Dtype> {
   public:
    explicit FixTPoolingLayer(const LayerParameter& param)
        : Layer<Dtype>(param){}
    virtual void LayerSetUp(const vector<Blob<Dtype>*>& bottom,
        const vector<Blob<Dtype>*>& top);
    virtual void Reshape(const vector<Blob<Dtype>*>& bottom,
        const vector<Blob<Dtype>*>& top);
    virtual inline const char* type() const { return "FixTPooling"; }
    virtual inline int ExactNumBottomBlobs() const { return 1; }
    virtual inline int ExactNumTopBlobs() const { return 1; }

  protected:
    virtual void Forward_cpu(const vector<Blob<Dtype>*>& bottom,
        const vector<Blob<Dtype>*>& top);
    virtual void Forward_gpu(const vector<Blob<Dtype>*>& bottom,
        const vector<Blob<Dtype>*>& top);
    virtual void Backward_cpu(const vector<Blob<Dtype>*>& top,
        const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);
    virtual void Backward_gpu(const vector<Blob<Dtype>*>& top,
        const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);

    int kernel_h_, kernel_w_, kernel_l_;
    int stride_h_, stride_w_, stride_l_;
    int pad_h_, pad_w_, pad_l_;
    int channels_;
    int height_, width_, length_;
    int pooled_height_, pooled_width_;
    int fix_time_;

     
};
}// namespace caffe 

#endif //CAFFE_FIX_TPOOLING_LAYER_HPP
