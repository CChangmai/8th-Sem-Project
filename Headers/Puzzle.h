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

#ifndef HAS_SIZE
        #define WIDTH 640
        #define HEIGHT 480
        #define BSIZE 80
        #define SIZE 48  
        #define HAS_SIZE  
#endif

const int BLOCKS_PER_ROW = 8;
const int BLOCKS_PER_COLUMN = 6;

#include<random>
#include<chrono>


unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//std::random_device seed;
std::default_random_engine generator(seed);
std::uniform_int_distribution<int> distribution(23,47);


/* 
   WE USE A RANDOM FUNCTION GENERATOR
   FROM C++ STANDARD LIBRARY
   TO GENERATE RANDOM NUMBERS

   WE WANT TO SWITCH IT IN A SEQUENCE
   SO WE DONT WANT NUMBERS TO GET REPEATED
   SO WE TAKE ONE HALF AS SEQUENTIAL
   AND OTHER HALF AS RANDOM
   SO THAT WHEN ONE HALF GETS SWAPPED
   THE SEQUENTIAL HALF DOESNT GET SWITCHED AGAIN
   EXAMPLE I CAN SWITCH FROM 0 TO 24 PIECE 
   WITH ANY RANDOM NUMBER FROM 24 TO 47
   I.E THE OTHER 24 PARTS
   BECAUSE
   IF SOMEHOW WE GET SAME NUMBER LIKE 0 OR 0
   OR PREVIOUSLY SWITCHED INDEX, LIKE SAY 3 OR 4
   WHEN SWITCHING WITH 5
   THIS ENSURES MINIMUM PRE-CORRECT PUZZLE LOCATIONS
*/   



//VARIABLE TO KEEP TRACK OF PUZZLE RECORDS
int records[SIZE];


cv::Mat Puzzle;

bool Assign_Puzzle(char* s,int bwidth,int bheight,int swidth,int sheight,cv::Scalar Color)
{
    cv::Mat image;
    image = cv::imread(s, CV_LOAD_IMAGE_COLOR);   // Read the file
      
    
    if(! image.data )                        // Check for invalid input
    {
        std::cout <<  "Could not open or find the image\n Please Enter A Valid Image Location\n";
        return false;
    }
    else
    {
    
      int i,j,count=0;
      
      cv::resize(image,image,cv::Size(WIDTH,HEIGHT));
      
      /*Copy Image To Puzzle After Resizing And Drawing Retangles*/
      
            for(i=0;i<HEIGHT;i+=bheight)
            {
            
                
                           
                    
                    for(j=0;j<WIDTH;j+=bwidth)
                    {
                    
                                                                     
                                        cv::rectangle(image,
                                        cv::Point(j,i),
                                        cv::Point(j+bwidth-1,j+bheight-1),
                                        Color,
                                        4,
                                        0.25,
                                        0);
                                                             
                                       
                            //KEEP TRACK OF BLOCK ASSIGNMENT 
                            records[count]=count;
                            count++;
                    }              
                            
            
            }
      
      /*
             cv::rectangle(image,cv::Point(0,400),cv::Point(80,479),Color,4,0.25,0);
             cv::rectangle(image,cv::Point(80,400),cv::Point(160,479),Color,4,0.25,0);
             cv::rectangle(image,cv::Point(160,400),cv::Point(240,479),Color,4,0.25,0);
             cv::rectangle(image,cv::Point(240,400),cv::Point(320,479),Color,4,0.25,0);
             cv::rectangle(image,cv::Point(320,400),cv::Point(400,479),Color,4,0.25,0);        
      */
      Puzzle=image.clone();
     
    
    std::cout<<"\n Puzzle Has Been Accepted : "<<count+1<<"\n";
    
    }
    
   return true;
    
}

bool isPuzzleComplete()
{
    for(int i=0;i<48;i++)
    {
            if(records[i] != i)
            {
              return false;
            }
    
    }

   return true;

}

void Puzzle_Rearrange()
{
  /*if(Puzzle.empty()!=1)
    {
  */  
    /* 
       THIS IS MUCH EASIER TO DO SINCE I KNOW HOW MANY DIVISIONS CAN BE MADE
       IN A ROW OR COLUMN
       AS COMPARED TO FINDING LINEAR INDEXES IN GPU
       SO NO ROW OR COLUMN FINDING PROBLEMS
       I NEED TEMPX AND TEMPY BECAUSE THE NORMAL
       FORMULA DOESN'T WORK FOR FIRST ROW
    */
    
          
        for(int i=0;i<(SIZE/2);i++)
        {
          
            // ASSUMING THAT THE RECORDS HAVE BEEN SET ALREADY 
            // DURING PUZZLE ASSIGNMENT
            
            int SWAP = distribution(generator); //GET A NUMBER FROM RANDOM NUMBER GENERATOR
            
            std::cout<<"Iteration : "<<i+1<<std::endl<<"Number Generated : "<<SWAP<<"\n";
            
           
            /* SWAP THE PUZZLE PIECES
               ONE PIECE IS INDICATED
               BY THE CURRENT ITERATION 
               IN THE LOOP
               NEXT WILL BE GENERATED BY THE RANDOM 
               NUMBER GENERATOR
            */
            
            
            cv::Rect temp1((SWAP % BLOCKS_PER_ROW) * BSIZE,
                           (SWAP / BLOCKS_PER_ROW) * BSIZE,
                            BSIZE,BSIZE);
            
            cv::Rect temp2( (i % BLOCKS_PER_ROW) * BSIZE,
                            (i / BLOCKS_PER_ROW) * BSIZE,
                             BSIZE,BSIZE);
                            
            //HOLD THE IMAGE BLOCK LOCATED IN RANDOM INDEX
            //AND SEQUENCE INDEX
            cv::Mat temp_img1 = Puzzle(temp1).clone();
            cv::Mat temp_img2 = Puzzle(temp2).clone();
            
            /*
               SWAP BOTH PICTURES WITH EACH OTHER
               OPENCV ONLY SUPPORTS copyTo
               ASSIGNMENT
               
            */
            
            temp_img1.copyTo(Puzzle(temp2));
            temp_img2.copyTo(Puzzle(temp1));
            /*
               KEEP A RECORD OF THE CHANGE/SWAP
            */    
                
                records[i]=SWAP;
                records[SWAP]=i;
                
            /*
              IF AFTER SWAPPING THE PLACES ARE CORRECT
              THEN DO HIGHLIGHTING OF THE COLORS
            */      
                
          
             
            
    
        }

   // }

}

void Swap_Pieces(int &prev_index,int &new_index,int block_size)
{
   
    /*
    Using This Concept
    
    If I know the number of block points that can be fit in one row,
    Then I can easily calculate The Number Of Rows It takes
    To Reach That Value  
    
    Turns out that in order to find the block we Need To Interchange the Sizes
    
    Number Of Blocks That can fit in to one row = columns/block_size
    Number Of Blocks That can fit in to one column = row/block_size
  */
  
  
  int old_x=(int)( ( prev_index % BLOCKS_PER_ROW ) * block_size );
  int old_y=(int)( ( prev_index / BLOCKS_PER_ROW ) * block_size );
  
  int temp;
 
  int new_x= (int)( ( new_index % BLOCKS_PER_ROW ) * block_size );
  int new_y= (int)( ( new_index / BLOCKS_PER_ROW ) * block_size );
  
 
  cv::Rect old_block(old_x,old_y,block_size,block_size);
  cv::Rect new_block(new_x,new_y,block_size,block_size);

  cv::Mat temp_old = Puzzle(old_block).clone();
  cv::Mat temp_new = Puzzle(new_block).clone();
  
  temp_old.copyTo( Puzzle(new_block) );
  temp_new.copyTo( Puzzle(old_block) );  
  
  //UPDATE PUZZLE RECORD
  
  temp=records[prev_index];
  records[prev_index]=records[new_index];
  records[new_index]=temp;
  
  
  
    if(records[prev_index]==prev_index)
            {
                cv::rectangle(Puzzle,
                              old_block,
                              cv::Scalar(0,255,0),
                              4,
                              0.25,
                              0);
            
            }   
            
      else
            {
                 cv::rectangle(Puzzle,
                              old_block,
                              cv::Scalar(0,0,255),
                              4,
                              0.25,
                              0);
            
            
            
            
            
            
            }
            
    if(records[new_index]==new_index)
           {
                cv::rectangle(Puzzle,
                               new_block,
                               cv::Scalar(0,255,0),
                               4,
                               0.25,
                               0);
            
            
            
            }
     else
     {
                         cv::rectangle(Puzzle,
                               new_block,
                               cv::Scalar(0,0,255),
                               4,
                               0.25,
                               0);
     
     
     
     
     
     }       


}

void Blend_Puzzle(cv::Mat &pic,float transparency=0.75)
{
    if(pic.data)
    {
    float opacity=1-transparency;
    addWeighted(Puzzle,opacity,pic,transparency,0.0,pic);
    }
}

void Draw_Single(int index,cv::Scalar Color)
{

int x=(int)( ( index % BLOCKS_PER_ROW ) * BSIZE );
int y=(int)( ( index / BLOCKS_PER_ROW ) * BSIZE );

cv::Rect paint_block(x,y,BSIZE-1,BSIZE-1);

                 cv::rectangle(Puzzle,
                               paint_block,
                               Color,
                               4,
                               0.25,
                               0);


}




