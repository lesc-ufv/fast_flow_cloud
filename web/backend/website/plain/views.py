from django.http import HttpResponse
from django.shortcuts import render
import json

# Create your views here.

def exec_code(request):
    result = json.loads(request.GET.get("data"))
    print(result['project_name'])
    print(result['compile_flags'])
    print(result['dataflows'])
    print(result['cgra_arch'])
    
    fileCpp = open('codes_cpp/main.cpp', 'w')
    fileCpp.write(result['sources']['main.cpp'])
    fileCpp.close()
    print("File .cpp created with success!")

    resposta = HttpResponse(json.dumps({'resposta': 'OK'}, separators=(',', ':')), content_type='application/json; charset=utf-8')
    resposta.status_code = 200
    resposta["Access-Control-Allow-Origin"] = "*"
    return resposta
