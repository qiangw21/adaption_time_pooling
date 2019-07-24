# adaption_time_pooling
- 实现在时间维度上自适应pooling,基于[video_caffe](https://github.com/chuckcho/video-caffe).
- 在caffe.proto pooling_param添加参数fix_time，为对于任意时间长度输入池化后输出的指定时间长度.
```
layer {
  name: "name"
  type: "FixTPooling"
  bottom: "bottomname"
  top: "topname"
  pooling_param {
    kernel_size: 3
    stride: 2
    fix_time: 24
  }
}
```
