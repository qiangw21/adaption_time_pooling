#include <algorithm>
#include <cfloat>
#include <vector>

#include "caffe/layers/fix_tpooling_layer.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

using std::min;
using std::max;

template <typename Dtype>
void FixTPoolingLayer<Dtype>::LayerSetUp(const vector<Blob<Dtype>*>& bottom,
     const vector<Blob<Dtype>*>& top) {
    PoolingParameter pool_param = this->layer_param_.pooling_param();
    CHECK(pool_param.has_fix_time())
        <<"Must input fix_time. ";
    CHECK(!pool_param.has_kernel_size() !=
        !(pool_param.has_kernel_h() && pool_param.has_kernel_w()))
        << "Filter size is kernel_size OR kernel_h and kernel_w; not both";
    CHECK(pool_param.has_kernel_size() ||
        (pool_param.has_kernel_h() && pool_param.has_kernel_w()))
        << "For non-square filters both kernel_h and kernel_w are required.";
    CHECK((!pool_param.has_pad() && pool_param.has_pad_h()
        && pool_param.has_pad_w())
        || (!pool_param.has_pad_h() && !pool_param.has_pad_w()))
        << "pad is pad OR pad_h and pad_w are required.";
    CHECK((!pool_param.has_stride() && pool_param.has_stride_h()
        && pool_param.has_stride_w())
        || (!pool_param.has_stride_h() && !pool_param.has_stride_w()))
        << "Stride is stride OR stride_h and stride_w are required.";
    fix_time_=pool_param.fix_time();
    if(pool_param.has_kernel_size()){
        kernel_h_ = kernel_w_ = pool_param.kernel_size();
    } else
    {
        kernel_h_ = pool_param.kernel_h();
        kernel_w_ = pool_param.kernel_w();
    }
    CHECK_GT(kernel_h_, 0) << "Filter dimensions cannot be zero.";
    CHECK_GT(kernel_w_, 0) << "Filter dimensions cannot be zero.";
    if (!pool_param.has_pad_h()) {
        pad_h_ = pad_w_ =pad_l_ = pool_param.pad();
    } else {
        pad_h_ = pool_param.pad_h();
        pad_w_ = pool_param.pad_w();
    }
    if (!pool_param.has_stride_h()) {
        stride_h_ = stride_w_ = pool_param.stride();
    } else {
        stride_h_ = pool_param.stride_h();
        stride_w_ = pool_param.stride_w();
    }
    if (pad_h_ != 0 || pad_w_ != 0) {
        CHECK(this->layer_param_.pooling_param().pool()
            == PoolingParameter_PoolMethod_AVE
            || this->layer_param_.pooling_param().pool()
            == PoolingParameter_PoolMethod_MAX)
            << "Padding implemented only for average and max pooling.";
        CHECK_LT(pad_h_, kernel_h_);
        CHECK_LT(pad_w_, kernel_w_);
    }
}

template <typename Dtype>
void FixTPoolingLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top) {
    CHECK(bottom[0]->num_axes() == 4 || bottom[0]->num_axes() == 5) << "Input "
        << "must have 4 or 5 axes, corresponding to (num, channels, [optional "
        << "length,] height, width)";
    channels_ = bottom[0]->channels();
    length_ = bottom[0]->length();
    height_ = bottom[0]->height();
    width_ = bottom[0]->width();
    pooled_height_ = static_cast<int>(ceil(static_cast<float>(
        height_ + 2 * pad_h_ - kernel_h_) / stride_h_)) + 1;
    pooled_width_ = static_cast<int>(ceil(static_cast<float>(
        width_ + 2 * pad_w_ - kernel_w_) / stride_w_)) + 1;
    if (pad_h_ || pad_w_) {
        // If we have padding, ensure that the last pooling starts strictly
        // inside the image (instead of at the padding); otherwise clip the last.
        if ((pooled_height_ - 1) * stride_h_ >= height_ + pad_h_) {
            --pooled_height_;
        }
        if ((pooled_width_ - 1) * stride_w_ >= width_ + pad_w_) {
            --pooled_width_;
        }
        CHECK_LT((pooled_height_ - 1) * stride_h_, height_ + pad_h_);
        CHECK_LT((pooled_width_ - 1) * stride_w_, width_ + pad_w_);
    }
    top[0]->Reshape(bottom[0]->num(),channels_,fix_time_,pooled_height_,
        pooled_width_);
}

template <typename Dtype>
void FixTPoolingLayer<Dtype>::Forward_cpu(
    const vector<Blob<Dtype>* >& bottom,
    const vector<Blob<Dtype>* >& top) {
  NOT_IMPLEMENTED;
}

template <typename Dtype>
void FixTPoolingLayer<Dtype>::Backward_cpu(
    const vector<Blob<Dtype>* >& bottom,
    const vector<bool>& propagate_down,
    const vector<Blob<Dtype>* >& top) {
  NOT_IMPLEMENTED;
}

#ifdef CPU_ONLY
STUB_GPU(PoolingLayer);
#endif

INSTANTIATE_CLASS(FixTPoolingLayer);
REGISTER_LAYER_CLASS(FixTPooling);

} //namespace caffe
