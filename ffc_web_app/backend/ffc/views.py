from django.http import HttpResponse
from django.shortcuts import render
import json

from xready_api.xready import XReady
# Create your views here.

def exec_code(request):
    result = json.loads(request.GET.get("data"))
    xready = XReady(result)
    out = xready.execute()
    response = HttpResponse(json.dumps({'response': out}, separators=(',', ':')), content_type='application/json; charset=utf-8')
    response.status_code = 200
    response["Access-Control-Allow-Origin"] = "*"
    return response
