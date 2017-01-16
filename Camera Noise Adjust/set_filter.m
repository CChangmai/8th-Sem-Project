function filtering_intensity = set_filter( image )
%This Function Basically Sorts Out How Aggressive A Filter To Use When 
%

[row col]=size(image);
%Use Laplacian To Filter Out The Image
l=fspecial('laplacian',0.4);
ig=rgb2gray(image);
inew=imfilter(ig,l,'conv');
ibw=gray2bin(inew);
%This is getting too Large for a small value Calculation

end
