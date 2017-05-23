#ifndef HAS_IOSTREAM
    #include<iostream>
    #include<vector>
    #include<algorithm>  // Mainly For Swapping Puzzle Pieces
    #define HAS_IOSTREAM
#endif
    
#ifndef HAS_OPENCV
    #include<opencv2/core/core.hpp>
    #include<opencv2/highgui/highgui.hpp>
    #include<opencv2/imgproc/imgproc.hpp>
    #define HAS_OPENCV
    
#endif    


cv::Mat Puzzle;

void Assign_Puzzle(char* s,int &bwidth,int &bheight,int &swidth,int &sheight,cv::Scalar &Color)
{
    cv::Mat image;
    image = cv::imread(s, CV_LOAD_IMAGE_COLOR);   // Read the file
    
    
    if(! image.data )                        // Check for invalid input
    {
        std::cout <<  "Could not open or find the image" <<std::endl ;
    }
    else
    {
    
      int i,j;
      cv::resize(image,image,cv::Size(swidth,sheight));
      
      /*Copy Image To Puzzle After Resizing And Drawing Retangles*/
            for(i=0;i<sheight;i+=bheight)
            {
                    
                    for(j=0;j<swidth;j+=bwidth)
                    {
                            cv::rectangle(image,
                                          cv::Point(j,i),
                                          cv::Point(j+bwidth,j+bheight),
                                          Color,
                                          8,
                                          0.25,
                                          0);
                  
            
                    }
            }
        
      
      Puzzle=image.clone();
     
    
    std::cout<<"\n Puzzle Has Been Accepted \n ";
    
    }
    
   
    
}

void Swap_Pieces(int &prev_index,int &new_index,int block_size)
{
  int height=Puzzle.rows;
  int width= Puzzle.cols;        
        
  cv::Mat temp;
  
  /*
    Using This Concept
    
    If I know the number of block points that can be fit in one row,
    Then I can easily calculate The Number Of Rows It takes
    To Reach That Value  
    
    Turns out that in order to find the block we Need To Interchange the Sizes
    
    Number Of Blocks That can fit in to one row = columns/block_size
    Number Of Blocks That can fit in to one column = row/block_size
  */
  
  
  int old_x=(int)((height/block_size) * prev_index);
  int old_y= (int)((width/block_size) * prev_index);
  
 
  int new_x=(int)((height/block_size) * new_index);
  int new_y= (int)((width/block_size) * new_index);
  
 
  cv::Rect old_block(old_x,old_y,block_size,block_size);
  cv::Rect new_block(new_x,new_y,block_size,block_size);

  temp=Puzzle(old_block);
  Puzzle(old_block)=Puzzle(new_block);
  Puzzle(new_block)=temp;  


}

void Blend_Puzzle(cv::Mat &pic,float transparency=0.75)
{
    float opacity=1-transparency;
    addWeighted(Puzzle,opacity,pic,transparency,0.0,pic);
}




