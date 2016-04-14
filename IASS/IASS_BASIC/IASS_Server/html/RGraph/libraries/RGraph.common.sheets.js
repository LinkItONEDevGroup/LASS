// version: 2016-02-06
    /**
    * o--------------------------------------------------------------------------------o
    * | This file is part of the RGraph package - you can learn more at:               |
    * |                                                                                |
    * |                          http://www.rgraph.net                                 |
    * |                                                                                |
    * | RGraph is dual licensed under the Open Source GPL (General Public License)     |
    * | v2.0 license and a commercial license which means that you're not bound by     |
    * | the terms of the GPL. The commercial license starts at just £99 (GBP) and      |
    * | you can read about it here:                                                    |
    * |                                                                                |
    * |                      http://www.rgraph.net/license                             |
    * o--------------------------------------------------------------------------------o
    */

    /**
    * Initialise the various objects
    */
    RGraph = window.RGraph || {isRGraph: true};

// Module pattern
(function (win, doc, undefined)
{
    RGraph.Sheets = function (key)
    {
        var worksheet,
            callback,
            letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';

        // 3 arguments
        if (arguments.length === 3) {
            worksheet = Number(arguments[1]);
            callback  = arguments[2];
        
        // 2 arguments
        } else {
            worksheet = 1;
            callback  = arguments[1];
        }
        
        var url = 'https://spreadsheets.google.com/feeds/cells/[KEY]/[WORKSHEET]/public/full?alt=json-in-script&callback=__rgraph_JSONPCallback'.replace(/\[KEY\]/, key).replace(/\[WORKSHEET\]/, worksheet);
        
        /*
        * https://spreadsheets.google.com/feeds/cells/1q_BMjvKO_kKbAO3VjoaITSDyrLAk8f0SK5UFMmE3oRs/2/public/full?alt=json-in-script
        */
    

        
        

        //
        // Loads the spreadsheet
        //
        this.load = function(url, userCallback)
        {
            var obj = this;

            // A global on purpose
            __rgraph_JSONPCallback = function (json)
            {
                // Save the JSON on the RGraph.Sheets object
                obj.json = json;

                //
                // Put the entries in the JSON feed into a grid
                //
                var grid = [], row = 0, col = 0;

                for (var i=0; i<json.feed.entry.length; ++i) {
                    
                    row = json.feed.entry[i].gs$cell.row - 1;
                    col = json.feed.entry[i].gs$cell.col - 1;
                    
                    if (!grid[row]) {
                        grid[row] = [];
                    }
                        
                    grid[row][col] = json.feed.entry[i].content.$t;
                }
                
                
                //
                // Determine the longest row
                //
                var maxcols = 0; // The max length of the rows
                
                for (var i=0; i<grid.length; ++i) {
                    maxcols = grid[i] ? Math.max(maxcols, grid[i].length) : maxcols;
                }


            
                //
                // Now go through the array and fill in any blank holes.
                //
                for (var i=0; i<grid.length; ++i) {
                    
                    if (typeof grid[i] === 'undefined') {
                        grid[i] = new Array(maxcols);
                    }

                    for (var j=0; j<maxcols; j++) {
                        if (typeof grid[i][j] === 'undefined') {
                            grid[i][j] = '';
                        }
                        
                        // Convert numbers to real numbers and floats here too
                        if (grid[i][j].match(/^[0-9]+$/)) {
                            grid[i][j] = parseInt(grid[i][j]);
                        } else if (grid[i][j].match(/^[0-9.]+$/)) {
                            grid[i][j] = parseFloat(grid[i][j]);
                        }
                    }
                }
                
                //
                // Save the grid on the object
                //
                obj.data = grid;



                //
                // Call the users callback function. Users can access the raw data if
                // they want with the obj.data property or by using the accessor
                // methods (better)
                //
                userCallback(obj);
            };

            // Add the new script tag to the document that pulls in the JSON
            //
            // With jQuery...
            //
            //$('body').append("<script src='" + url + "'></script>");
            //
            // And without jQuery...

            var scriptNode = document.createElement('SCRIPT');
            scriptNode.src = url;
            document.body.appendChild(scriptNode);
        };




        //
        // Fetches a row of data and returns it
        //
        // @param id    number The numeric index of the column to fetch (starts at 1)
        // @param start number The index to start fetching/returning at. The first
        //                     character is 1
        // @param opt        An option object containing options
        //
        this.row = function (index, start)
        {
            var opt = {}, row;
            
            // Default for start is 1
            start = start || 1;

            //
            // Parse the .trim argument
            //

            if (arguments && typeof arguments[2] === 'object' && typeof arguments[2].trim === 'boolean') {
                opt.trim = arguments[2].trim;
            } else {
                opt.trim = true;
            }

            row = this.data[index - 1].slice(start - 1);

            // Trim the array if required
            if (opt.trim) {
                row = RGraph.arrayTrim(row);
            }

            return row;
        };
        
        
        
        //
        // Fetches a column of data and returns it
        //
        // @param id    number The letter that corresponds to the column
        // @param start number The index to start fetching/returning at. The first
        //                     character is 1
        // @param opt        An option object containing options
        //
        this.col = function (index, start)
        {
            var opt = {},
                col = [];
          
            // Default for start is 1
            start = start || 1;

            if (arguments && typeof arguments[2] === 'object' && typeof arguments[2].trim === 'boolean') {
                opt.trim = arguments[2].trim;
            } else {
                opt.trim = true;
            }
            
            for (var i=0; i<this.data.length; ++i) {
                col.push(this.data[i][index - 1]);
            }

            // Trim the array if required
            if (opt.trim) {
                col = RGraph.arrayTrim(col);
            }

            // Now account for the start index

            col = col.slice(start - 1);

            return col;
        };
        
        
        
        //
        // Returns the index (zero index) of the given letters
        //
        this.getIndexOfLetters = function (l)
        {
            var parts = l.split('');

            if (parts.length === 1) {
                return letters.indexOf(l) + 1;
            } else if (parts.length === 2){
                var idx = ((letters.indexOf(parts[0]) + 1) * 26) + (letters.indexOf(parts[1]) + 1);
                return idx;
            }
        }




    
        //
        // The get method makes retrieving cells very straightforward,
        // for example: obj.get('B1');
        //
        // @param str string The cells(s) to fetch
        // @param     string Optional set of options that are passed
        //                   to the relevant row/col function
        //
        this.get = function (str)
        {
            // Uppercase letters please!
            str = str.toUpperCase();

            //
            // Handle the style of .get('C') or .get('AA'
            //
            if (str.match(/^[a-z]+$/i)) {
                if (str.length === 1) {
                    var index = letters.indexOf(str) + 1;
                    return this.col(index, 1, arguments[1]);
                
                } else if (str.length === 2) {
                    var index = ((letters.indexOf(str[0]) + 1) * 26) + letters.indexOf(str[1]) + 1;
                    return this.col(index, 1, arguments[1]);
                }
            }
            
            
            
            
            //
            // Handle the style of .get('2');
            //(fetching a whole row
            //
            if (str.match(/^[0-9]+$/i)) {
                return this.row(str, null, arguments[1]);
            }
            
            
            
            
            //
            // Handle the style of .get('E2');
            //(fetching a single cell)
            //
            if (str.match(/^([a-z]{1,2})([0-9]+)$/i)) {
                
                var letter = RegExp.$1,
                    number = RegExp.$2,
                    col    = this.get(letter, {trim: false});

                
                return col[number - 1];
            }
            
            
            
            
            //
            // Handle the style of .get('B2:E2');
            //(fetching the E2 cell to the E2 cell)
            //
            if (str.match(/^([a-z]{1,2})([0-9]+):([a-z]{1,2})([0-9]+)$/i)) {

                var letter1 = RegExp.$1,
                    number1 = RegExp.$2,
                    letter2 = RegExp.$3,
                    number2 = RegExp.$4


                if (letter1 === letter2) {
                    var cells = [],
                        index = this.getIndexOfLetters(letter1),
                        col   = this.col(index, null, {trim: false});

                    for (var i=(number1 - 1); i<=(number2 - 1); ++i) {
                        cells.push(col[i]);
                    }

                } else if (number1 === number2) {

                    var cells = [],
                        row   = this.row(number1, null, {trim: false}),
                        index1 = this.getIndexOfLetters(letter1),
                        index2 = this.getIndexOfLetters(letter2)

                    for (var i=(index1 - 1); i<=(index2 - 1); ++i) {
                        cells.push(row[i]);
                    }
                }

                // Trim the results
                if (arguments[1] && arguments[1].trim === false) {
                    // Nada
                } else {
                    cells = RGraph.arrayTrim(cells);
                }

                return cells;
               
            }
        };




        //
        // Load the data
        //
        this.load(url, callback);
    };

// End module pattern
})(window, document);