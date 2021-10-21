import '../css/style.scss';
import 'bootstrap';
import 'popper.js';
import 'bootstrap/scss/bootstrap.scss';
import { cy, nodeTypes } from './cinit';
import exportModule from './modules/export';
import adjustLayoutModule from './modules/adjustLayout';
// import ElCounter from './js/utils/idGenerator';
import importModule from './modules/import';
import contextMenusModule from './modules/contextMenus';
import copyModule from './modules/copy';
import { saveAs } from 'file-saver';

import 'brace';
import 'brace/mode/c_cpp';
import 'brace/theme/dawn';

// initialize the content of the text editor to some C++
$("#editor").text(`#include <iostream>\n#include <ffc.h>\n\nusing namespace std;\nint main(int argc, char *argv[]){\n\tFFC::getInstance()->setArgs(argc,argv);\n\tcout << "Hello from Fast Flow Cloud!" << endl;\n\treturn 0;\n}`);
    
// initialize the editor environment using the ace library
var editor = ace.edit("editor");
editor.session.setMode("ace/mode/c_cpp"); // editor language
editor.setTheme("ace/theme/dawn"); // editor theme
editor.session.setTabSize(4);
editor.session.setUseWrapMode(true);
$("#editor").css("fontSize", "15px");

contextMenusModule(cy, nodeTypes);

cy.edgehandles({
  noEdgeEventsInDraw: true,
  start(sourceNode) {
    cy.$('node[numOperands>0]')
      .filter(
        node =>
          node.indegree() >= node.data('numOperands') && !node.same(sourceNode),
      )
      .addClass('invalid');
  },
  stop() {
    cy.$('node[numOperands>0]').removeClass('invalid');
  },
  edgeType(sourceNode, targetNode) {
    if (targetNode.indegree() >= targetNode.data('numOperands')) {
      return null;
    }
    if (targetNode.hasClass('parent')) {
      return null;
    }
    return 'flat';
  },

  edgeParams(sourceNode, targetNode, _) {
    if (targetNode.data('commutative') === false) {
      // console.log('incomers', targetNode.incomers('edge').data());
      const opNum = targetNode.incomers('edge').data('opNum') === 1 ? 2 : 1;

      return {
        data: {
          opNum,
        },
        classes: ['operand'],
      };
    }
    return {};
  },

  complete(sourceNode, targetNode, addedEles) {
    if (targetNode.hasClass('reducible') && targetNode.indegree() > 2)
      targetNode.addClass('reduce');
    // console.log(targetNode.data());
    if (targetNode.data('commutative') === false) {
      console.log('commute', addedEles.data(), addedEles.classes());
    }
  },
});

$('#import').click(() => {
  importModule(cy, nodeTypes);
});

$('#adjust-layout').click(() => {
  adjustLayoutModule(cy);
});

copyModule(cy);

$("#btn-save").click(function(event){
   event.preventDefault();
   if($("#dfg-name").val() == ""){
       alert("Dataflow name cannot be empty!");
   }
   else{
       $("#graph-tab").text("Dataflow: " + $("#dfg-name").val());
       $("#btn-close").click();
   }
});

function parseURLParams(url) {
    var queryStart = url.indexOf("?") + 1,
        queryEnd   = url.indexOf("#") + 1 || url.length + 1,
        query = url.slice(queryStart, queryEnd - 1),
        pairs = query.replace(/\+/g, " ").split("&"),
        parms = {}, i, n, v, nv;

    if (query === url || query === "") return;

    for (i = 0; i < pairs.length; i++) {
        nv = pairs[i].split("=", 2);
        n = decodeURIComponent(nv[0]);
        v = decodeURIComponent(nv[1]);

        if (!parms.hasOwnProperty(n)) parms[n] = [];
        parms[n].push(nv.length === 2 ? v : null);
    }
    return parms;
}

function newId(listNodes, oldId){
    for (let index = 0; index < listNodes.length; index++) {
        if(listNodes[index].old_id == oldId){
            return listNodes[index].id;
        }
    }
}

function getCyNode(id,nodes){
     for (let index = 0; index < nodes.length; index++) {
        if(nodes[index].data.id == id){
            return nodes[index].data;
        }
    }
    return {'type':'fake'};
}

function getNode(id,nodes){
     for (let index = 0; index < nodes.length; index++) {
        if(nodes[index].id == id){
            return nodes[index];
        }
    }
    return {'type':'fake'};
}

function standardizeDfgJson(json) {
    var res = {
        "id": 0,
        "name": $("#graph-tab").text().replace(/\s/g, '').split(":")[1],
        "nodes": [],
        "edges": []
    };
    var nodes = [];
    var edges = [];
    var contNodes = 0;
    var contEdges = 0;
    var consts = {};
    for (let index = 0; index < json.length; index++) {
        if(json[index].group == 'nodes' && json[index].data.type != 'const' && json[index].data.type){
            if(json[index].data.type == 'input'){
            nodes.push({
                "id": contNodes,
                "old_id": json[index].data.id,
                "in_id": json[index].data.in_id,
                "label": json[index].data.type,
                "opcode": json[index].data.type,
            });
            }else if (json[index].data.type == 'output'){
                            nodes.push({
                "id": contNodes,
                "old_id": json[index].data.id,
                "out_id": json[index].data.out_id,
                "label": json[index].data.type,
                "opcode": json[index].data.type,
            });
            }else{
             nodes.push({
                "id": contNodes,
                "old_id": json[index].data.id,
                "label": json[index].data.type,
                "opcode": json[index].data.type,
                "const_id": -1,
            });
            }
            contNodes++;
        }else if(json[index].group == 'edges'){
            var n = getCyNode(json[index].data.source,json);
            if(n.type != 'fake'){
                if( n.type != 'const'){
                    edges.push({
                        "port": 0,
                        "source": json[index].data.source,
                        "target": json[index].data.target
                    });
                    contEdges++;
                }else{
                    consts[json[index].data.target] = n;
                }
            }
        }
    }
    for (let index = 0; index < edges.length; index++) {
        edges[index].source = newId(nodes, edges[index].source);
        edges[index].target = newId(nodes, edges[index].target);
    }
    //FIXME: the user need choice the port to conncect on the node.
    var port_edges={}
    for (let index = 0; index < edges.length; index++) {
         var key = edges[index].target+"-"+edges[index].port
         if(key in port_edges){
                edges[index].port = 1;
         }else{
             port_edges[key] = true;
         }
    }

    res.edges = edges;
    for(var c in consts){
        for (let index = 0; index < nodes.length; index++){
            if(c == nodes[index].old_id){
                nodes[index].const_id = consts[c].cid;
            }
        }
    }
    for (let index = 0; index < nodes.length; index++) {
        var n = {
            "id": nodes[index].id,
            "label": nodes[index].label,
            "opcode": nodes[index].opcode,
            "const_id": nodes[index].const_id,
        }
        if(n.label == 'input'){
            n['in_id'] = nodes[index].in_id;
        }else if(n.label== 'output'){
            n['out_id'] = nodes[index].out_id;
        }
        res.nodes.push(n);
    }
    return res;
}

$("#settings").click((event) => {
    event.preventDefault();
    $('#modal-settings').modal({backdrop: 'static', keyboard: false});
});

$('#run').click((event) => {
    event.preventDefault();
    var jsonDfg = exportModule(cy);
    var outputCode = editor.getValue();
    if(jsonDfg.length == 0){
        alert("Please, draw a valid dataflow!");
    }else if(outputCode == ""){
        alert("Please, make a C++ code first!");
    }else{
        $("#run").prop("disabled", true);
        $("#icon-run").removeClass("fa fa-play");
        $("#icon-run").addClass("fa fa-spinner fa-spin fa-3x fa-fw");
        var finalJson = {
            "project_name": parseURLParams(location.href).project_name[0],
            "sources": {
                "main.cpp": outputCode
            },
            "compile_flags": ["-std=c++11", "-fopenmp","-lxready"],
            "dataflows": [standardizeDfgJson(jsonDfg)],
            "cgra_name": "cgra_128_8_8_16",
            "run_mode": $("input[name=type_run]:checked").val()
        }
        $.ajax({ 
            type: "GET",
            url: 'http://54.144.151.112:8000/exec_code',
            data: {
                "data": JSON.stringify(finalJson)
            },
            dataType: 'json',
            success: function(result){
                $("#log").val(result.response);
                $('#modal-result').modal({backdrop: 'static', keyboard: false});
                $("#run").prop("disabled", false);
                $("#icon-run").removeClass("fa fa-spinner fa-spin fa-3x fa-fw");
                $("#icon-run").addClass("fa fa-play");
            },
            fail: function(msg){
                console.log("ERRO 1");
            },
            beforeSend: function(){
            },
            complete: function(msg){
            },
            error: function(msg){
                console.log("ERRO 2");
            }
        });
    }
});

$("#adjust-layout").mouseover(function(event){
   event.preventDefault();
   $(".label-adjust").css("opacity", "1"); 
});

$("#adjust-layout").mouseleave(function(event){
    event.preventDefault();
    $(".label-adjust").css("opacity", "0"); 
});

$("#run").mouseover(function(event){
    event.preventDefault();
    $(".label-run").css("opacity", "1"); 
});
 
$("#run").mouseleave(function(event){
    event.preventDefault();
    $(".label-run").css("opacity", "0"); 
});

$("#settings").mouseover(function(event){
    event.preventDefault();
    $(".label-settings").css("opacity", "1"); 
 });
 
 $("#settings").mouseleave(function(event){
     event.preventDefault();
     $(".label-settings").css("opacity", "0"); 
 });
