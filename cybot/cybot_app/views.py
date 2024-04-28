from django.shortcuts import render

# Create your views here.
def radar_display(request):
    return render(request, 'radar.html')
