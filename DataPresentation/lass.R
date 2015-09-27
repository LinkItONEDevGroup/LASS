gpsPlot <- function(){

#reference : http://rgraphgallery.blogspot.tw/2013/04/rg68-get-google-map-and-plot-data-in-it.html
# loading the required packages
#please use Package Installer to install these 2 module, remind to checked dependence when install
require(ggplot2)
require(ggmap)

data = read.table("ride2.csv",sep = ",",header = TRUE)
# creating a sample data.frame with your lat/lon points
#lon <- c(120.99307166666667)
#lat <- c(24.79977166666667)
lon <- data[,7]
lat <- data[,6]

df <- as.data.frame(cbind(lon,lat))

# getting the map
mapgilbert <- get_map(location = c(lon = mean(df$lon), lat = mean(df$lat)), zoom = 14,
                      maptype = "roadmap", scale = 2) #"roadmap, satellite

# plotting the map with some points on it
ggmap(mapgilbert) +
  geom_point(data = df, aes(x = lon, y = lat, fill = "red", alpha = 0.8), size = 5, shape = 21) +
  guides(fill=FALSE, alpha=FALSE, size=FALSE)
}