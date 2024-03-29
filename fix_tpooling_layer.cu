#include <algorithm>
#include <cfloat>
#include <vector>

#include "caffe/layers/fix_tpooling_layer.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

template <typename Dtype>
__global__ void FixAvePoolForward(const int nthreads,const Dtype* const bottom_data,const int num,
    const int channels,const int length,const int height,const int width,const int pooled_length,
    const int pooled_height,const int pooled_width,const int kernel_l,const int kernel_h,
    const int kernel_w,const int stride_l,const int stride_h,const int stride_w,const int pad_l,
    const int pad_h,const int pad_w,Dtype* const top_data){
        CUDA_KERNEL_LOOP(index,nthreads) {
            const int pw=index % pooled_width;
            const int ph=(index / pooled_width) % pooled_height;
            const int pl=(index / pooled_width / pooled_height) % pooled_length;
            const int c=(index / pooled_width / pooled_height / pooled_length) % channels;
            const int n=index / pooled_width / pooled_height / pooled_length / channels;

            //int redundant_length=(stride_l+1)*pooled_length-length;
            int redundant_length=pooled_length-length%pooled_length;
            int lstart,lend;
            if(pl>=redundant_length){
                lstart=pl*stride_l+pl-redundant_length-pad_l;
                lend=min(lstart+kernel_l+1,length+pad_l);
            }else{
                lstart=pl*stride_l-pad_l;
                lend=min(lstart+kernel_l,length+pad_l);
            }

            
            int hstart=ph*stride_h-pad_h;
            int wstart=pw*stride_w-pad_w;
            
            int hend=min(hstart+kernel_h,height+pad_h);
            int wend=min(wstart+kernel_w,width+pad_w);
            const int pool_size=(lend-lstart)*(hend-hstart)*(wend-wstart);
            
            lstart=max(lstart,0);
            hstart=max(hstart,0);
            wstart=max(wstart,0);
            lend=min(lend,length);
            hend=min(hend,height);
            wend=min(wend,width);
            Dtype aveval = 0;
            const Dtype* const bottom_slice=bottom_data + (n*channels+c)*length*height*width;
            for(int l=lstart;l<lend;++l){
                for(int h=hstart;h<hend;++h){
                    for(int w=wstart;w<wend;++w){
                        aveval+=bottom_slice[(l*height+h)*width+w];
                    }
                }
            }
            top_data[index] = aveval / pool_size;

        }
    }

template <typename Dtype>
void FixTPoolingLayer<Dtype>::Forward_gpu(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
    const Dtype* bottom_data =bottom[0]->gpu_data();
    Dtype* top_data = top[0]->mutable_gpu_data();
    int count = top[0]->count();
    length_=bottom[0]->length();
    stride_l_=kernel_l_=length_/fix_time_;

    FixAvePoolForward<Dtype><<<CAFFE_GET_BLOCKS(count), CAFFE_CUDA_NUM_THREADS>>>(
        count,bottom_data,bottom[0]->num(),channels_,length_,height_,width_,
        fix_time_,pooled_height_,pooled_width_,kernel_l_,kernel_h_,kernel_w_,stride_l_,
        stride_h_,stride_w_,pad_l_,pad_h_,pad_w_,top_data);
    

}

template <typename Dtype>
__global__ void FixAvePoolBackward(const int nthreads,const Dtype* const top_diff,const int num,
    const int channels,const int length,const int height,const int width,const int pooled_length,
    const int pooled_height,const int pooled_width,const int kernel_l,const int kernel_h,
    const int kernel_w,const int stride_l,const int stride_h,const int stride_w,const int pad_l,
    const int pad_h,const int pad_w,Dtype* const bottom_diff) {
        CUDA_KERNEL_LOOP(index,nthreads){
            const int w = index % width + pad_w;
            const int h = (index / width) % height + pad_h;
            const int l = (index / width / height) % length + pad_l;
            const int c = (index / width / height / length) % channels;
            const int n = index / width / height /length / channels;

            int redundant_length=pooled_length-length%pooled_length;

            int plstart,plend;
            if(l>=stride_l*redundant_length){
                plstart=redundant_length+
                    (l-redundant_length*stride_l<kernel_l+1)?0:(l-redundant_length*stride_l-kernel_l-1)/(stride_l+1)+1;
                plend=min(redundant_length+(l-stride_l*redundant_length)/(stride_l+1)+1,pooled_length);
            }else{
                plstart=(l<kernel_l)?0:(l-kernel_l)/stride_l+1;
                plend=min(l/stride_l+1,pooled_length);
            }
            const int phstart = (h < kernel_h) ? 0 : (h - kernel_h) / stride_h + 1;
            const int phend = min(h / stride_h + 1, pooled_height);
            const int pwstart = (w < kernel_w) ? 0 : (w - kernel_w) / stride_w + 1;
            const int pwend = min(w / stride_w + 1, pooled_width);
            Dtype gradient = 0;
            const Dtype* const top_diff_slice=top_diff+(n*channels+c)*pooled_length
                    *pooled_height*pooled_width;

            for(int pl=plstart;pl<plend;++pl){
                for(int ph=phstart;ph<phend;++ph){
                    for(int pw=pwstart;pw<pwend;++pw){
                        int lstart,lend;
                        if(pl>=redundant_length){
                            lstart=pl*stride_l+pl-redundant_length-pad_l;
                            lend=min(lstart+kernel_l+1,length+pad_l);
                        }else{
                            lstart=pl*stride_l-pad_l;
                            lend=min(lstart+kernel_l,length+pad_l);
                        }
                        int hstart=ph*stride_h-pad_h;
                        int wstart=pw*stride_w-pad_w;
            
                        int hend=min(hstart+kernel_h,height+pad_h);
                        int wend=min(wstart+kernel_w,width+pad_w);
                        int pool_size=(lend-lstart)*(hend-hstart)*(wend-wstart);
                        gradient += top_diff_slice[(pl*pooled_height+ph)*pooled_width+pw] / pool_size;
                    }
                }
            }
            bottom_diff[index] = gradient;
        }
}

template <typename Dtype>
void FixTPoolingLayer<Dtype>::Backward_gpu(const vector<Blob<Dtype>*>& top,
    const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom){
        if(!propagate_down[0]){
            return;
        }
        const Dtype* top_diff = top[0]->gpu_diff();
        Dtype* bottom_diff=bottom[0]->mutable_gpu_diff();
        const int count = bottom[0]->count();
        length_=bottom[0]->length();
        stride_l_=kernel_l_=length_/fix_time_;

        FixAvePoolBackward<Dtype><<<CAFFE_GET_BLOCKS(count), CAFFE_CUDA_NUM_THREADS>>>(
            count,top_diff,top[0]->num(),channels_,length_,height_,width_,fix_time_,
            pooled_height_,pooled_width_,kernel_l_,kernel_h_,kernel_w_,stride_l_,stride_h_,stride_w_,
            pad_l_,pad_h_,pad_w_,bottom_diff );
}

INSTANTIATE_LAYER_GPU_FUNCS(FixTPoolingLayer);


}//namespace caffe

