import math
from django.http import JsonResponse

# Helper function to convert polar coordinates to Cartesian
def polar_to_cartesian(degrees, distance):
    radians = degrees * (math.pi / 180)
    x = distance * math.cos(radians)
    y = distance * math.sin(radians)
    return {'x': x, 'y': y}

# Converts an array of polar coordinates to Cartesian coordinates
def convert_polar_to_cartesian_array(polar_coordinates):
    cartesian_coordinates = []
    for degrees, distance in polar_coordinates:
        cartesian = polar_to_cartesian(degrees, distance)
        cartesian_coordinates.append(cartesian)
    return cartesian_coordinates

# Rotate points based on the given angle and direction
def rotate_points(polar_coordinates, angle, direction):
    rotated_coordinates = [
        (((degrees - angle) if direction == 'right' else (degrees + angle)) % 360, distance)
        for degrees, distance in polar_coordinates
    ]
    return convert_polar_to_cartesian_array(rotated_coordinates)

# Django view to handle rotation and point calculation
def handle_rotate(request):
    polar_coordinates = request.GET.get('polar_coordinates', [])
    angle = float(request.GET.get('angle', 0))
    direction = request.GET.get('direction', 'left')

    # Parsing the string of coordinates to a list of tuples
    polar_coordinates = [tuple(map(float, coord.split(','))) for coord in polar_coordinates.split(';')]

    # Calculate rotated points
    rotated_points = rotate_points(polar_coordinates, angle, direction)

    # Return the calculated points as JSON
    return JsonResponse(rotated_points, safe=False)

# Example usage in urls.py
# path('rotate/', handle_rotate, name='rotate')


