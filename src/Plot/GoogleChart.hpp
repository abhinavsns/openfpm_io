/*
 * GoogleChart.hpp
 *
 *  Created on: Jan 9, 2016
 *      Author: i-bird
 */

#ifndef OPENFPM_DATA_SRC_PLOT_GOOGLECHART_HPP_
#define OPENFPM_DATA_SRC_PLOT_GOOGLECHART_HPP_

#include <fstream>
#include "Vector/map_vector.hpp"
#include <cmath>

#define GGRAPH_COLUMS 1
#define GGRAPH_POINTS 2

#define GC_ZOOM std::string("explorer: {actions: ['dragToZoom', 'rightClickToReset'],axis: 'horizontal,vertical',keepInBounds: true, maxZoomIn: 128.0}")
#define GC_X_LOG std::string("hAxis: { logScale: true }")
#define GC_Y_LOG std::string("vAxis: { logScale: true }")

/*! \brief Google chart options
 *
 */
struct GCoptions
{
	//! Title of the chart
	std::string title;
	//! Y axis name
	std::string yAxis;
	//! X axis name
	std::string xAxis;

	//! Type of chart (list of the option can be founded in Google Chart API for seriesType)
	//! Possible options are:
	//!  'line', 'area', 'bars', 'candlesticks', and 'steppedArea'
	//! default: line
	std::string stype;

	//! Extended series options
	//! Example {5: {type: 'line'}} specify that the series number 5 must be represented
	//! with a line
	std::string stypeext;

	//! width of the graph in pixels
	size_t width=900;

	//! height of the graph in pixels
	size_t heigh=500;

	//! Flag that specify if the colums are stacked
	//! Check in Google Chart for is stacked option
	bool isStacked = false;

	//! Width of the line
	size_t lineWidth = 4;

	//! Style for all the intervals
	//! Check Google Chart API intervals option
	std::string intervalsext;

	//! Style for each interval
	//! Check Google Chart API interval option
	std::string intervalext;

	//! more
	std::string more;

	//! curve type
	std::string curveType = "function";

	//! barWD
	bool barWD = false;

	/*! \brief copy operator
	 *
	 * \param opt object to copy
	 *
	 * \return itself
	 *
	 */
	GCoptions & operator=(const GCoptions & opt)
	{
		title = opt.title;
		yAxis = opt.yAxis;
		xAxis = opt.xAxis;
		stype = opt.stype;
		stypeext = opt.stypeext;
		width=opt.width;
		heigh=opt.heigh;

		lineWidth = opt.lineWidth;
		intervalsext = opt.intervalsext;
		more = opt.more;

		return *this;
	}
};

/*! \brief Google Graph
 *
 */
struct GGraph
{
	//! TypeOfGraph
	size_t type;

	//! data
	std::string data;

	//! option
	std::string option;

	//! view in case we need a view
	std::string view;

	//! Google chart option
	GCoptions opt;
};

/////////////////// Constants strings usefull to construct the HTML page //////////

const std::string begin_data ="<html>\n\
  <head>\n\
    <script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>\n\
	<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script>\n\
    <script type=\"text/javascript\">\n\
      google.charts.load('current', {'packages':['corechart']});\n\
      google.charts.setOnLoadCallback(drawVisualization);\n\
\n\
function exportToSVG(i)\n\
{\n\
var e = document.getElementById('chart_div'+i);\n\
var svg = e.getElementsByTagName('svg')[0].parentNode.innerHTML;\n\
var pos = svg.lastIndexOf(\"</svg>\");\n\
pos += 6;\n\
svg = svg.substring(0,4)  + \" xmlns='http://www.w3.org/2000/svg' xmlns:xlink= 'http://www.w3.org/1999/xlink' \" + svg.substring(4,pos);\n\
svgData = 'data:image/svg+xml;charset=utf-8,' + encodeURIComponent(svg);\n\
$(this).attr({'href': svgData,'target': '_blank'});\n\
}\n\
\n\
      function drawVisualization() {\n";

const std::string end_data="]);\n\n";

const std::string begin_div = "}</script>\n\
</head>\n\
<body>\n";

const std::string div_end = "</body>\n\
</html>\n";

const std::string saving_javascript = "function save(i)\n\
										var e = document.getElementById('chart_')\n\
										e.getElementsByTagName('svg')[0].parentNode.innerHTML";

template<typename T>
struct check_nan
{
	static bool check(const T & n)
	{
		return false;
	}
};

template<>
struct check_nan<float>
{
	static bool check(const float & n)
	{
		return std::isnan(n);
	}
};

template<>
struct check_nan<double>
{
	static bool check(const double & n)
	{
		return std::isnan(n);
	}
};

/////////////////////////////////////////////////////////////////////

/*! \brief Small class to produce graph with Google chart in HTML
 *
 * This Class can produce several graph using google chart
 *
 * ### Create Histogram graph
 *
 * \image html g_graph_hist.jpg
 *
 * This code produce the graph above
 *
 * \snippet Plot_unit_tests.hpp Producing an Histogram graph
 *
 * ### Create Lines
 *
 * \image html g_graph_plot2.jpg
 *
 * This code produce the graph above
 *
 * \snippet Plot_unit_tests.hpp Producing lines
 *
 * ### Create lines with different styles
 *
 * \image html g_graph_plot.jpg
 *
 * This code produce the graph above
 *
 * \snippet Plot_unit_tests.hpp Producing lines graph with style
 *
 */
class GoogleChart
{
	template<typename arg_0, typename ... args>
	struct get_value_type
	{
		typedef typename arg_0::value_type type;
	};

	//! set of graphs
	openfpm::vector<GGraph> set_of_graphs;

	//! set inject HTML;
	openfpm::vector<std::string> injectHTML;

	//! Data has holes
	bool holes = false;

	/*! \brief Recursively sort variadic template of vectors
	 *
	 * \param x vector with x
	 * \param y vector with y
	 * \param xy all the other vectors
	 *
	 */
	template<typename X, typename ... Xs>
	void recursive_sort(X & x,X & y,Xs& ... xy)
	{
		struct srt_xy
		{
			typename X::value_type x;
			unsigned int id;

			bool operator<(const srt_xy & tmp) const
			{
				return x < tmp.x;
			}
		};

		openfpm::vector<srt_xy> reord;
		X y_tmp;

		reord.resize(x.size());
		y_tmp.resize(x.size());

		for (size_t i = 0 ; i < x.size() ; i++)
		{
			reord.get(i).x = x.get(i);
			reord.get(i).id = i;
		}

		reord.sort();

		// reorder x and y

		for (size_t i = 0 ; i < x.size() ; i++)
		{
			x.get(i) = reord.get(i).x;
			y_tmp.get(i) = y.get(reord.get(i).id);
		}

		y_tmp.swap(y);

		// sort x

		recursive_sort(xy ...);
	}

	//! terminator for recursive sort
	void recursive_sort()
	{}

	/*! \brief Recursively sort variadic template of vectors
	 *
	 * \param counters indexes
	 * \param x vector with x
	 * \param y vector with y
	 * \param xy all the other vectors
	 *
	 */
	template<typename X, typename ... Xs>
	bool isNext(size_t * counters,X & x,X & y,Xs& ... xy)
	{
		if (counters[0] < x.size())
		{
			return true;
		}
		return isNext(&counters[1],xy ...);
	}

	/*! \brief Recursively sort variadic template of vectors
	 *
	 * \param counters indexes
	 *
	 */
	bool isNext(size_t * counters)
	{
		return false;
	}


	/*! \brief Recursively sort variadic template of vectors
	 *
	 * \param counters indexes
	 * \param x vector with x
	 * \param y vector with y
	 * \param xy all the other vectors
	 *
	 */
	template<typename T, typename X, typename ... Xs>
	T get_low(size_t * counters,X & x,X & y,Xs& ... xy)
	{
		if (sizeof...(Xs) != 0)
		{
			T low1;
			if (counters[0] >= x.size())
			{low1 = std::numeric_limits<typename X::value_type>::infinity();}
			else
			{low1 = x.get(counters[0]);}

			T low2 = get_low<T>(&counters[1],xy ...);
			return (low1 < low2)?low1:low2;
		}

		if (counters[0] >= x.size())
		{return std::numeric_limits<typename X::value_type>::infinity();}
		return x.get(counters[0]);
	}

	/*! \brief Recursively sort variadic template of vectors
	 *
	 * \param counters indexes
	 *
	 */
	template<typename T>
	T get_low(size_t * counters)
	{
		return 0.0;
	}

	/*! \brief Recursively sort variadic template of vectors
	 *
	 * \param x vector with x
	 * \param y vector with y
	 * \param xy all the other vectors
	 *
	 */
	template<typename X, typename ... Xs>
	void get_point(typename X::value_type low,
			   typename X::value_type * point,
			   size_t * counters,
			   X & x,
			   X & y,
			   Xs& ... xy)
	{
		if (counters[0] < x.size() && low == x.get(counters[0]))
		{
			point[0] = y.get(counters[0]);
			counters[0]++;
		}
		else
		{
			point[0] = std::numeric_limits<typename X::value_type>::quiet_NaN();
		}

		get_point(low,&point[1],&counters[1],xy...);
	}

	/*! \brief Recursively sort variadic template of vectors
	 *
	 * \param x vector with x
	 * \param y vector with y
	 * \param xy all the other vectors
	 *
	 */
	template<typename T>
	void get_point(T low,
			   T * point,
			   size_t * counters)
	{
		return;
	}

	/*! \brief Recursively sort variadic template of vectors
	 *
	 * \param x vector with x
	 * \param y vector with y
	 * \param xy all the other vectors
	 *
	 */
	template<typename ... Xs>
	bool get_v(typename get_value_type<Xs...>::type & x,
			   typename get_value_type<Xs...>::type * point,
			   size_t * counters,
			   Xs& ... xy)
	{
		// if exist the next element
		if (isNext(counters,xy...) == false)
		{return false;}

		// get lowest x
		typename get_value_type<Xs...>::type low = get_low<typename get_value_type<Xs...>::type>(counters,xy...);

		x = low;

		get_point(low,point,counters,xy...);

		return true;
	}

	/*! \brief Given X and Y vector return the string representing the data section of the Google Chart
	 *
	 * \tparam X type for the X coordinates
	 * \tparam Y type for the Y coordinates
	 *
	 * \param x vector of points on x
	 * \param y vector of points on y
	 * \param yn vector containing the name of each graph
	 * \param opt options to draw the graph
	 * \param i index of the graph we are drawing
	 *
	 * \return string with the data section
	 *
	 */
	template<typename X, typename Y> std::string get_points_plot_data(const openfpm::vector<X> & x, const openfpm::vector<Y> & y, const openfpm::vector<std::string> & yn, const GCoptions & opt, size_t i)
	{
		std::stringstream data;

		size_t interval = 0;

		// we require that the number of x elements are the same as y elements

		if (x.size() != y.size())
		{std::cerr << "Error: " << __FILE__ << ":" << __LINE__ << " vector x and the vector y must have the same number of elements " << x.size() << "!=" << y.size() << "\n";}

		// Google chart visualization
        data << "var data" << i << " = new google.visualization.DataTable();\n";
		if (std::is_same<X,typename std::string>::value == true)
			data << "data" << i << ".addColumn("  << "'string'" << "," << "'" << opt.xAxis <<"');\n";
		else
			data << "data" << i << ".addColumn("  << "'number'" << "," << "'" << opt.xAxis <<"');\n";

        for (size_t j = 0 ; j < y.last().size() ; j++)
        {
        	if (yn.get(j) == std::string("interval"))
        	{
        		data << "data" << i << ".addColumn({id:'i" << interval/2 << "', type:'number', role:'interval'});\n";
        		interval++;
        	}
        	else
        		data << "data" << i << ".addColumn("  << "'number'" << "," << "'" << yn.get(j) <<"');\n";
        }

        data << "data" << i << ".addRows([\n";
        for (size_t i = 0 ; i < y.size() && x.size() ; i++)
        {

        	for (size_t j = 0 ; j < y.get(i).size()+1 ; j++)
        	{
        		// the first is x
        		if (j == 0)
        		{
        			if (std::is_same<X,typename std::string>::value == true)
        				data << "['"  << x.get(i) << "'";
        			else
        				data << "["  << x.get(i);
        		}
        		else
        		{
        			if (check_nan<typename Y::value_type>::check(y.get(i).get(j-1)) == false)
        			{
        				data << "," << y.get(i).get(j-1);
        			}
        			else
        			{
        				holes = true;
        				data << "," << "null";
        			}
        		}
        	}
        	data << "],\n";
        }

		return data.str();
	}

	/*! \brief Construct a view option
	 *
	 * \param opt GoogleChart option
	 *
	 * \return the string
	 *
	 */
	std::string get_view_bar_option(const GCoptions & opt, size_t n_col)
	{
		if (opt.barWD == false)
			return std::string();

		std::stringstream str;

		str << "[0" << std::endl;

		for (size_t i = 1 ; i < n_col ; i++)
		{
			str << "," << i << ",{ calc: \"stringify\"," << std::endl;
	        str << "sourceColumn: " << i << "," << std::endl;
	        str << "type: \"string\"," << std::endl;
	        str << "role: \"annotation\" }"<< std::endl;
		}

		str << "]" << std::endl;

	    return str.str();
	}

	std::string get_colums_bar_option(const GCoptions & opt)
	{
		std::stringstream str;
		str << "title : '" << opt.title << "'";
	    str << ",\nvAxis: {title: '" << opt.yAxis <<  "'}";
	    str << ",\nhAxis: {title: '" << opt.xAxis << "'}";
	    str << ",\nseriesType: '" << opt.stype << "'";
	    if (opt.stypeext.size() != 0)
	    	str << ",\nseries: " << opt.stypeext;
	    if (opt.more.size() != 0)
	    	str << ",\n" <<opt.more;

	    return str.str();
	}

	std::string get_points_plot_option(const GCoptions & opt)
	{
		std::stringstream str;
		str << "title : '" << opt.title << "'";
	    str << ",\nvAxis: {title: '" << opt.yAxis <<  "'}";
	    str << ",\nhAxis: {title: '" << opt.xAxis << "'}";
	    str << ",\ncurveType: '"<< opt.curveType << "'";

        str << ",\nlineWidth: " << opt.lineWidth;
        if (opt.intervalsext.size() != 0)
        	str << ",\nintervals: " << opt.intervalsext;
        else
        	str << ",\nintervals: " << "{ 'style':'area' }";

        if (opt.intervalext.size() != 0)
        	str << ",\ninterval: " << opt.intervalext << "\n";

        if (opt.more.size() != 0)
        	str << ",\n" << opt.more;

	    return str.str();
	}

	/*! \brief Add a graph data variable
	 *
	 * \param of file out
	 * \param i id
	 * \param data string
	 *
	 */
	void addData(std::ofstream & of, size_t i, const std::string & data)
	{

		of << data;
		of << "]);\n";
	}

	/*! \brief Add an option data variable
	 *
	 * \param of file out
	 * \param i id
	 * \param opt string
	 *
	 */
	void addOption(std::ofstream & of, size_t i, const std::string & opt)
	{
		of << "var options";
		of << i;
		of << "= {\n";
		if (holes == true)
		{of << "interpolateNulls : true,\n";}
		of << opt;
		of << "};\n";
	}

	/*! \brief Add a view data variable
	 *
	 * \param of file out
	 * \param i id
	 * \param view string
	 *
	 */
	void addView(std::ofstream & of, size_t i, std::string view)
	{
		if (view.size() == 0)
			return;

		of << "var view" << i << " = new google.visualization.DataView(data" << i << ");" << std::endl;
		of << "view"<< i << ".setColumns(";
		of << view << ");" << std::endl;
	}

	/*! \brief Add a draw div section
	 *
	 * \param of file out
	 * \param i id
	 * \param draw_view draw a chart(true) or view(false)
	 *
	 */
	void addDrawDiv(std::ofstream & of, size_t i, bool draw_view)
	{
		of << "$(\"#export_svg" << i << "\").on(\"click\", function (event) {exportToSVG.apply(this,[" << i << "]);});\n";
		of << "var chart = new google.visualization.ComboChart(document.getElementById('chart_div";
		of << i;
		of << "'));" << std::endl;
		if (draw_view == true)
		{
			of << "chart.draw(data";
			of << i;
		}
		else
		{
			of << "chart.draw(view";
			of << i;
		}
		of << ", options";
		of << i;
		of << ");\n";
	}

	/*! \brief Add a div section
	 *
	 * \param of file ofstream
	 * \param i id of the graph
	 * \param gc GoogleChart option
	 *
	 */
	void addDiv(std::ofstream & of, size_t i, const GCoptions & gc)
	{
		of << "<a href=\"#\" download=\"graph1.svg\" id=\"export_svg" << i << "\"><button>Export data into svg</button></a>";
		of << "<div id=\"chart_div";
		of << i;
		of << "\" style=\"width: ";
		of << gc.width;
		of << "px; height: ";
		of << gc.heigh;
		of << "px;\"></div>\n";
	}

public:

	GoogleChart()
	{
		injectHTML.add();
	}

	/*! \brief Add an histogram graph
	 *
	 * \param y A vector of vectors the size of y indicate how many values we have on x
	 *          each x value can have multiple values or datasets
	 *
	 */
	template<typename Y> void AddHistGraph(openfpm::vector<Y> & y)
	{
		openfpm::vector<std::string> x;
		x.resize(y.size());

		AddHistGraph<std::string,Y>(x,y);
	}

	/*! \brief Add an histogram graph
	 *
	 * \param y A vector of vectors the size of y indicate how many values we have on x
	 *          each x value can have multiple values or datasets
	 *
	 * \param x Give a name or number to each colums. Can be a string or a number
	 *
	 */
	template<typename X, typename Y> void AddHistGraph(openfpm::vector<X> & x, openfpm::vector<Y> & y)
	{
		GCoptions opt;

		openfpm::vector<std::string> yn;

		if (y.size() != 0)
			yn.resize(y.get(0).size());

		AddHistGraph<X,Y,std::string>(x,y,yn,opt);
	}

	/*! \brief Add an histogram graph
	 *
	 * \param y A vector of vectors the size of y indicate how many values we have on x
	 *          each x value can have multiple values or datasets
	 *
	 * \param x Give a name or number to each colums. Can be a string or a number
	 *
	 * \param yn Give a name to each dataset
	 *
	 */
	template<typename X, typename Y, typename Yn> void AddHistGraph(openfpm::vector<X> & x, openfpm::vector<Y> & y, openfpm::vector<Yn> & yn)
	{
		GCoptions opt;

		AddHistGraph(x,y,yn,opt);
	}

	/*! \brief Add an histogram graph
	 *
	 * \param y A vector of vectors the size of y indicate how many values we have on x
	 *          each x value can have multiple values or datasets
	 *
	 * \param x Give a name or number to each colums. Can be a string or a number
	 *
	 * \param yn Give a name to each dataset
	 *
	 * \param opt Graph options
	 *
	 */
	template<typename X, typename Y, typename Yn> void AddHistGraph(openfpm::vector<X> & x, openfpm::vector<Y> & y, openfpm::vector<Yn> & yn , const GCoptions & opt)
	{
		set_of_graphs.add();
		injectHTML.add();

		// Check that all the internal vector has the same number of elements

		if (y.size() != 0)
		{
			size_t sz = y.get(0).size();
			for (size_t i = 0; i < y.size() ; i++)
			{
				if (y.get(i).size() != sz)
					std::cerr << __FILE__ << ":" << __LINE__ << " error all the elements in the y vector must have the same numbers, element " << i << ": " << y.get(i).size() << " " << " mismatch the numbers of elements at 0: " << sz << "/n";
			}
		}

		set_of_graphs.last().type = GGRAPH_COLUMS;
		set_of_graphs.last().data = get_points_plot_data(x,y,yn,opt,set_of_graphs.size()-1);
		set_of_graphs.last().option = get_colums_bar_option(opt);
		set_of_graphs.last().view = get_view_bar_option(opt,y.get(0).size());
		set_of_graphs.last().opt = opt;
	}


	/*! \brief Add lines graph
	 *
	 * \param xy list of vectors like openfpm::vector<float> or openfpm::vector<double>. Suppose you have a dataset of points x1,y1
	 *           and a dataset x2,y2 and you want to display all these points in one graph. Than you call this function with
	 *           AddLines(x1,y1,x2,y2,opt)
	 *
	 * \param opt Graph options
	 *
	 */
	template<typename ... X> void AddLines(const openfpm::vector<std::string> & yn,
										   const GCoptions & opt,
										   X ... xy)
	{
		// first we sort the vectors
		recursive_sort(xy ... );

		size_t counters[sizeof...(X)];
		memset(&counters,0,sizeof(counters));

		typename get_value_type<X...>::type point[sizeof...(X)];
		typename get_value_type<X...>::type xe;

		openfpm::vector<typename get_value_type<X...>::type> x;

		openfpm::vector<openfpm::vector<typename get_value_type<X...>::type>> y;


		while (get_v(xe,point,counters,xy...))
		{
			y.add();
			x.add(xe);

			for (size_t i = 0 ; i < sizeof...(X)/2 ; i++)
			{
				y.last().add(point[i]);
			}
		}

		AddLinesGraph(x,y,yn,opt);
	}

	/*! \brief Add lines graph
	 *
	 * If the x vector contain 0.0,0.1,0.2,0.3,0.4 and you want to draw two lines you specifying two y values for each of the x points
	 * you create an openfpm::vector<openfpm::vector<float>> y such that
	 *
	 *  \verbatim
                               ------  j  -------        |
	   y.get(i).get(j) = 3.2   3.4    5.4    1.3    2.3  i
	                     1.4   5.3    3.2    2.1    1.1  |

	   \endverbatim
	 *
	 * \param y A vector of vectors of values. each vector contain the graph points, compared to AddLinesGraph the points are stored
	 *
	 *
	 * \param x axis values
	 *
	 * \param opt Graph options
	 *
	 */
	template<typename X, typename Y> void AddLinesGraphT(openfpm::vector<X> & x, openfpm::vector<Y> & y , const GCoptions & opt)
	{
		openfpm::vector<std::string> yn;

		if (y.size() == 0)
		{
			std::cerr << "Error: " << __FILE__ << ":" << __LINE__ << " vector y must be filled" << std::endl;
			return;
		}

		for (size_t i = 0 ; i < y.last().size() ; i++)
			yn.add(std::string("line") + std::to_string(i));

		if (y.size() == 0)
			return;

		// number of points
		size_t np = y.last().size();

		for (size_t j = 0 ; j < y.size() ; j++)
		{
			if (y.get(j).size() != np)
				std::cerr << __FILE__ << ":" << __LINE__ << " Error all the graph must have the same number of points " << np << "!=" << y.get(j).size() << std::endl;
		}

		openfpm::vector<openfpm::vector<X>> swap;


		// swap the vector
		// Each vector is a graph
		// It is different from the other call where each vector
		// has multiple value for the same point
		for (size_t i = 0 ; i < np ; i++)
		{
			swap.add();
			for (size_t j = 0 ; j < y.size() ; j++)
			{
				swap.last().add(y.get(j).get(i));
			}
		}

		AddLinesGraph(x,swap,yn,opt);
	}

	/*! \brief Add a simple lines graph
	 *
	 * If the x vector contain 0.0,0.1,0.2,0.3,0.4 and you want to draw two lines you specifying two y values for each of the x points
	 * you create an openfpm::vector<openfpm::vector<float>> y such that
	 *
	 *  \verbatim
                         --  j  --
	   y.get(i).get(j) = 3.2   3.4
	                     1.4   5.3 |
	                     5.4   3.2 i
	                     1.3   2.2 |
                         2.3   1.1
	   \endverbatim
	 *
	 * \param y A vector of vectors of values. The size of y indicate how many x values
	 *          we have, while the internal vector can store multiple value of the same point,
	 *          for example error bar
	 *
	 * \param x Give a name or number to each x value, so can be a string or a number
	 *
	 * \param opt Graph options
	 *
	 */
	template<typename X, typename Y> void AddLinesGraph(openfpm::vector<X> & x, openfpm::vector<Y> & y , const GCoptions & opt)
	{
		openfpm::vector<std::string> yn;

		if (y.size() == 0)
		{
			std::cerr << "Error: " << __FILE__ << ":" << __LINE__ << " vector y must be filled";
			return;
		}

		for (size_t i = 0 ; i < y.last().size() ; i++)
			yn.add(std::string("line") + std::to_string(i));

		AddLinesGraph(x,y,yn,opt);
	}

	/*! \brief Add a simple plot graph
	 *
	 * \param y A vector of vector of values (numbers) the size of y indicate how many x values
	 *          or colums we have, while the internal vector store multiple lines,
	 *          or error bars
	 *
	 * \param x Give a name or number to each colums, so can be a string or a number
	 *
	 * \param yn Give a name to each line, or specify an error bar
	 *
	 * \param opt Graph options
	 *
	 */
	template<typename X, typename Y> void AddLinesGraph(openfpm::vector<X> & x, openfpm::vector<Y> & y , const openfpm::vector<std::string> & yn, const GCoptions & opt)
	{
		if (y.size() == 0)
		{
			std::cerr << "Error: " << __FILE__ << ":" << __LINE__ << " vector y must be filled\n";
			return;
		}

		set_of_graphs.add();
		injectHTML.add();

		// Check that all the internal vectors has the same number of elements

		if (y.size() != 0)
		{
			size_t sz = y.get(0).size();
			for (size_t i = 0; i < y.size() ; i++)
			{
				if (y.get(i).size() != sz)
					std::cerr << __FILE__ << ":" << __LINE__ << " error all the elements in the y vector must have the same numbers of elements " << y.get(i).size() << " != " << sz << "\n";
			}
		}

		set_of_graphs.last().type = GGRAPH_POINTS;
		set_of_graphs.last().data = get_points_plot_data(x,y,yn,opt,set_of_graphs.size()-1);
		set_of_graphs.last().option = get_points_plot_option(opt);
		set_of_graphs.last().opt = opt;
	}

	/*! \brief Add HTML text
	 *
	 * \param html add html text in the page
	 *
	 */
	void addHTML(const std::string & html)
	{
		injectHTML.last() = html;
	}

	/*! \brief It write the graphs on file in html format using Google charts
	 *
	 * \param file output file
	 *
	 */
	void write(std::string file)
	{
		// Open a file

		std::ofstream of(file);

		// Check if the file is open
		if (of.is_open() == false)
		{std::cerr << "Error cannot create the HTML file: " + file + "\n";}

		// write the file

		of << begin_data;

		for (size_t i = 0 ; i < set_of_graphs.size() ; i++)
			addData(of,i,set_of_graphs.get(i).data);

		for (size_t i = 0 ; i < set_of_graphs.size() ; i++)
			addOption(of,i,set_of_graphs.get(i).option);

		for (size_t i = 0 ; i < set_of_graphs.size() ; i++)
			addView(of,i,set_of_graphs.get(i).view);

		for (size_t i = 0 ; i < set_of_graphs.size() ; i++)
			addDrawDiv(of,i,set_of_graphs.get(i).view.size() == 0);

		of << begin_div;

		of << injectHTML.get(0);

		for (size_t i = 0 ; i < set_of_graphs.size() ; i++)
		{
			addDiv(of,i,set_of_graphs.get(i).opt);
			of << injectHTML.get(i+1);
		}

		of << div_end;

		of.close();
	}
};

#endif /* OPENFPM_DATA_SRC_PLOT_GOOGLECHART_HPP_ */
