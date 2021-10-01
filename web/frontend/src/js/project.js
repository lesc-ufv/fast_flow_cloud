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
$("#editor").text(`#include <iostream>\n\nusing namespace std;\n\nint main(){\n\tcout << "Hello World!" << endl;\n\treturn 0;\n}`);
    
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

function standardizeDfgJson(json) {
    var res = {
        "id": 0,
        "name": $("#graph-tab").text().split(":")[1],
        "nodes": [],
        "edges": []
    };
    var nodes = [];
    var edges = [];
    var contNodes = 0;
    var contEdges = 0;
    for (let index = 0; index < json.length; index++) {
        if(json[index].group == 'nodes'){
            nodes.push({
                "id": contNodes,
                "old_id": json[index].data.id,
                "label": json[index].data.type,
                "opcode": json[index].data.type
            });
            contNodes++;
        }else if(json[index].group == 'edges'){
            edges.push({
                "port": contEdges,
                "source": json[index].data.source,
                "target": json[index].data.target
            });
            contEdges++;
        }
    }
    for (let index = 0; index < edges.length; index++) {
        edges[index].source = newId(nodes, edges[index].source);
        edges[index].target = newId(nodes, edges[index].target);
    }
    for (let index = 0; index < nodes.length; index++) {
        res.nodes.push({
            "id": nodes[index].id,
            "label": nodes[index].label,
            "opcode": nodes[index].opcode
        }); 
    }
    res.edges = edges;
    return res;
}

$('#run').click((event) => {
    event.preventDefault();
    var jsonDfg = exportModule(cy);
    var outputCode = editor.getValue();
    if(jsonDfg.length == 0){
        alert("Please, draw a valid dataflow!");
    }else if(outputCode == ""){
        alert("Please, make a c++ code first!");
    }else{
        var finalJson = {
            "project_name": parseURLParams(location.href).project_name[0],
            "sources": {
                "main.cpp": outputCode
            },
            "compile_flags": ["-std=c++11", "-fopenmp"],
            "dataflows": [standardizeDfgJson(jsonDfg)],
            "cgra_arch": {
                "id": 0,
                "num_pe": 128,
                "num_pe_in": 8,
                "num_pe_out": 8,
                "data_width": 16,
                "net_radix": 8,
                "extra_stagies": 1,
                "conf_depth": 1,
                "conf_bus_width": 8,
                "conf_bus_pipe": 1
            }
        }
        $.ajax({ 
            type: "GET",
            url: 'http://localhost:8000/exec_code',
            data: {
                "data": JSON.stringify(finalJson)
            },
            dataType: 'json',

            success: function(result){
                console.log(result);
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
