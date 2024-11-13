
@tool

class_name  chunk_roads_manager extends Node3D

@export var json_data : JSON 

@export_range(0.0001, 1, 0.0001) var curve_bake_interval : float = 0.005

#places all the roads into an array, NOT FILTERED
func get_all_roads(roads : JSON) -> Array:
	if roads == null: #check that a valid json has been input
		print("No Data, Have you added a Json?")
		return []
	#output the json data into a dictionary
	var roads_as_dict : Dictionary = roads.data 
	#output the features. features is an array of dictionaries with each dictionary having data for one road
	var array_of_roads : Array = roads_as_dict.get("features")
	return array_of_roads # will return a problem if json is badly formatted

#From a non filtered array of roads, Filter into a dictionary of {ROAD ID, [COORDS]}
func make_road_dictionary(array_of_roads : Array) -> Dictionary: #MANAGER 
	var all_roads_dictionary : Dictionary
	for i in array_of_roads:
		#get the ID, includes the type prefix
		var id : String = i.get("properties").get("@id")
		#Remove the type prefix for all way's
		id = id.trim_prefix("way/")
		#Add the coordinates ID and coordinates array as a data entry
		all_roads_dictionary.get_or_add( id , i.get("geometry").get("coordinates"))
	return all_roads_dictionary

#get the coordinates array of a single road
func get_road_coordinates(road_dict : Dictionary , roadid : String) -> Array:
	if road_dict.get(roadid) == null:
		"No Road With that Id found, returning blank array"
		return []
	var road_coordinates : Array = road_dict.get(roadid)
	return road_coordinates


#COORDINATE CALCULATIONS
static var earth_radius = 6378137.0 #floats aren't accurate enough in this binarary
func lat2z(lat) -> float: # breaks if above or below 89.5
	return log(tan(PI/4 + deg_to_rad(lat)/2 )) * earth_radius

func long2x(lon) -> float:
	return deg_to_rad(lon) * earth_radius

func translate_latlong_to_xy(Coordinates : Array , origin_coords : Array) -> Array:
	var translated_xy : Array
	var origin_coords_translated : Array = [long2x(origin_coords[0]),lat2z(origin_coords[1])]
	for coords in Coordinates:
		translated_xy.append( [ snapped( long2x(coords[0]) - origin_coords_translated[0] , 0.01) , -1 * snapped( lat2z(coords[1]) - origin_coords_translated[1], 0.01 ) ] )
	print(translated_xy)
	return translated_xy

#return the coords of the origin, around a point on a single road
func find_origin_from_single_road_point(roads_dict : Dictionary, origin_road_id : String , origin_point_index : int) -> Array:
	var origin_coords : Array
	if roads_dict == null:
		print("roads_dict is null, Have you added a Json?")
		return []
	var road_coordinates : Array = get_road_coordinates(roads_dict,origin_road_id)
	origin_coords = road_coordinates[origin_point_index]
	return origin_coords



#place all the roads from a dict, returns an array of road nodes (as nodes)
func place_all_road_nodes(roads_dict : Dictionary,origin_coords : Array) -> Array:
	if roads_dict == null:
		print("roads_dict is null, Have you added a Json?")
		return []
	var roads_nodes : Array[SingleRoadSectionJustNode]
	for id in roads_dict:
		#create road node
		var road := SingleRoadSectionJustNode.new()
		#get coordinates for points along road and place them in an array
		var road_coordinates : Array = get_road_coordinates(roads_dict,id)
		#add road to the array then as a child of the manager
		roads_nodes.append(road)
		add_child(road)
		
		#assign variables on road node
		road.road_id = id
		road.curve_bake_interval = curve_bake_interval
		
		#convert to cartesian in local frame
		road_coordinates = translate_latlong_to_xy(road_coordinates,origin_coords)
		#choose to create a smooth or non smooth curve for road to follow
		road.create_non_smooth_curve(road_coordinates)
		#road.construct_smooth_curve()

	return roads_nodes
#Delete all roads currently being managed
func delete_roads(road_nodes: Array[SingleRoadSectionJustNode]) -> void:
	for i in road_nodes:
		i.queue_free()
	road_nodes.clear()

func _ready() -> void:
	print("RANANDREADY")
	#clear any potential existing roads of this node
	for child in get_children():
		child.queue_free()
	var road_data_dict = make_road_dictionary(get_all_roads(json_data))
	print("\n----------------------------------\n")
	#array which contains a copy each road
	var _theroads = place_all_road_nodes(road_data_dict, find_origin_from_single_road_point(road_data_dict,"25067308",0))
