from django.urls import path
from .views import radar_display

urlpatterns = [
    path('', radar_display, name='home'),
]

