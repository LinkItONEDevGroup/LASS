/*
*
* Leaflet divHeatmap Layer
* Lightweight heatmap implementation using divIcons and CSS3 radial gradients
* 
* */


L.DivHeatmapLayer = L.FeatureGroup.extend({
  options: {
    color: '#00ff00',
    radius: 150,
    gradient: true,
    clickable: false
  },

  initialize: function(options){
    L.Util.setOptions(this, options);
    this._layers = {};

    // Define CSS rule for making heatmap invisible to clicking
    var style = document.createElement('style');
    document.head.appendChild(style);

    // Append CSS rules
    //style.appendChild(document.createTextNode(''));
    //var sheet = style.sheet;
    //sheet.insertRule(".leaflet-heatmap-blob { transition: width 1s, height 1s; }", 0);
  },

  _parseColor: function (color) {
    m = color.match(/^#([0-9a-f]{6})$/i);
    if(m) {
      return (parseInt(m[1].substr(0,2),16) + ',' + parseInt(m[1].substr(2,2),16)+ ',' + parseInt(m[1].substr(4,2),16));
    }else{
      return color;
    }
  },

  _addBlob: function(lat,lng,value,old_marker){
    // Remove previous
    if (typeof old_marker != 'undefined') {
      this.removeLayer(old_marker);
    }

    if ( (!value && value != 0) || (!lat && lat != 0) || (!lng && lng != 0)) {
      throw new Error('Provide a latitude, longitude and a value');
    }

    if (value > 1 || value < 0) {
      throw new Error('Value should be beetween 0 and 1');
    }

    // Define the marker
    var alpha_start = value,
    alpha_end = !this.options.gradient ? value : 0,
    opacity = value;

    //----------------------
    //modified by cclljj
    //----------------------
    opacity = 1;
    var c_r, c_g, c_b;

                                if (value < 0.12){
					c_r = 0;
					c_g = 255;
					c_b = 85;
                                } else if (value<0.24){
					c_r = 0;
					c_g = 255;
					c_b = 130;
                                } else if (value<0.36){
					c_r = 0;
					c_g = 170;
					c_b = 0;
                                } else if (value<0.42){
					c_r = 255;
					c_g = 255;
					c_b = 100;
                                } else if (value<0.48){
					c_r = 255;
					c_g = 170;
					c_b = 80;
                                } else if (value<0.54){
					c_r = 140;
					c_g = 100;
					c_b = 0;
                                } else if (value<0.59){
					c_r = 255;
					c_g = 160;
					c_b = 125;
                                } else if (value<0.65){
					c_r = 140;
					c_g = 85;
					c_b = 95;
                                } else if (value<0.71){
					c_r = 190;
					c_g = 65;
					c_b = 25;
                                } else {
					c_r = 255;
					c_g = 0;
					c_b = 255;
                                }


//    var gradient =  'radial-gradient(closest-side, rgba('+ this._parseColor(this.options.color) +', '+alpha_start+') 0%, rgba('+ this._parseColor(this.options.color) +', '+alpha_end+') 100%)';
    var gradient =  'radial-gradient(closest-side, rgba('+ c_r+','+c_g+','+c_b+', '+alpha_start+') 0%, rgba('+ c_r+','+c_g+','+c_b+', '+alpha_end+') 100%)';
    //----------------------
    // end of modification
    //----------------------

    var html = '<div class="heatblob" data-value="'+value+'" style="width:100%;height:100%;border-radius:50%;background-image:'+gradient+'">';
    var size = this.options.radius * value;
    var divicon = L.divIcon({
      iconSize: [ size, size ],
      className: 'leaflet-heatmap-blob',
      html: html
    });

    var marker = L.marker([lat, lng], {
      icon: divicon,
      clickable: this.options.clickable,
      keyboard: false,
      opacity: opacity
    }).addTo(this);

    return marker;
  },

  _dataset: [],
  _markerset: [],

  setData: function(data) {
    // Data object is three values [ {lat,lon,value}, {...}, ...]
    this.clearData();
    var self = this;
    data.forEach(function(point){
      point.value = point.value > 1 ? 1 : point.value;
      var marker = self._addBlob(point.lat,point.lon,point.value);
      self._markerset.push(marker);
      self._dataset.push({
        "lat": point.lat,
        "lon": point.lon,
        "value": point.value
      });

    });
  },

  getData: function() {
    return this._dataset;     
  },

  clearData: function() {
    this.clearLayers();
    this._markerset = [];
    this._dataset = [];
  },


  _animateBlob: function(lat,lng,start_value,end_value,marker,fadeIn,fadeOut) {
    var self = this;
    if (!marker) var marker;

    var v = start_value;
    var delay = 50; // millis
    var step = start_value < end_value ? 0.1 : -0.1;

    var seed = setInterval(function() {
      //if (!marker) self.removeLayer(marker);
      v = v + step;
      // Gate values so that the blob is always correct during progression
      v < 0 ? v = 0 : v > 1 ? v = 1 : v = v;
      marker = self._addBlob(lat,lng,v,marker);
      if (v >= Math.max(start_value,end_value) || v <= Math.min(start_value,end_value)) {
        //console.log(lat,lng,v,marker);
        window.clearInterval(seed);
        if (v <= Math.min(start_value,end_value)) {
          if (fadeOut) fadeOut(marker);
        }
        if (v >= Math.max(start_value,end_value)) {
          if (fadeIn) fadeIn(marker);
        }
      };
    },delay);

  },

  morphData: function(new_data) {
    this.fadeOutData();
    this.fadeInData(new_data);
  },

  fadeInData: function (data) {
    var self = this;
    data.forEach(function(point){
      point.value = point.value > 1 ? 1 : point.value;
      self._animateBlob(point.lat,point.lon,0,point.value,null,function fadeIn(marker) {
                self._markerset.push(marker);
        self._dataset.push({
          "lat": point.lat,
          "lon": point.lon,
          "value": point.value
        });
      });
    })
  },

  fadeOutData: function() {
    var self = this;
    var qty = self._dataset.length;
    for (var i =0; i < qty; i++) {
      var point = self._dataset.pop();
      var marker = self._markerset.pop();
      self._animateBlob(point.lat,point.lon,point.value,0,marker,null,function fadeOut(marker) {
        self.removeLayer(marker);
      });
    }
  },


  /*
  * Testing
  */
  testRandomData: function(number) {
    var data = [];
    var count = number || 100;
    while (count-- > 0) {
      var size = Math.round(Math.random() * 150);	// in pixels
      var value = Math.random();	// 0 - 1 (opacity)
      var lat = 90 * Math.random();
      var lng = 180 * Math.random();
      data.push({
        "lat": lat,
        "lon": lng,
        "value": value
      });
    }
    return data;
  },

  testAddPoints: function(number) {
    var count = number || 100;
    while (count-- > 0) {
      var size = Math.round(Math.random() * 150);	// in pixels
      var value = Math.random();	// 0 - 1 (opacity)
      var lat = 90 * Math.random();
      var lng = 180 * Math.random();
      this._addBlob(lat,lng,value);
    }
  },

  testAddData: function(number) {
    this.clearData();
    this.setData(this.testRandomData(number));
  },

  testAnimatePoints: function(number) {
    this.clearData();
    var self = this;
    var data = this.testRandomData(number);
    data.forEach(function(point) {
      self._animateBlob(point.lat,point.lon,0,point.value);
    });
  },

  testMorphData: function(number) {
    this.clearData();
    this.setData(this.testRandomData(number));
    this.morphData(this.testRandomData(number));
  },

 });

