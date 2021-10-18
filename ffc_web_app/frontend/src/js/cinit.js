import cytoscape from 'cytoscape';
import edgehandles from 'cytoscape-edgehandles';
import contextMenus from 'cytoscape-context-menus';
import clipboard from 'cytoscape-clipboard';
import undoRedo from 'cytoscape-undo-redo';
import dagre from 'cytoscape-dagre';
import nodeHtmlLabel from 'cytoscape-node-html-label';
import $ from 'jquery';
import nodeTypes from './modules/nodeTypes';
import styles from '../assets/json/styles.json';

cytoscape.use(contextMenus, $);
cytoscape.use(clipboard, $);
cytoscape.use(edgehandles);

undoRedo(cytoscape);

cytoscape.use(dagre);

nodeHtmlLabel(cytoscape);

const initconfig = {
  container: document.getElementById('cy'),
  layout: {
    name: 'dagre',
    fit: false,
    maxSimulationTime: 1000,
  },
  style: [...nodeTypes.styles, ...styles],
};

const cy = cytoscape(initconfig);

cy.filter(node => node.hasClass('reducible') && node.indegree() > 2).addClass(
  'reduce',
);

cy.on('mouseover', 'node,edge', evt => {
  const element = evt.target;
  element.toggleClass('selected');
});

cy.on('mouseout', 'node,edge', evt => {
  const element = evt.target;
  element.toggleClass('selected');
});

cy.on('click', 'node', evt => {
  evt.target.incomers().forEach(elem => {
      elem.removeClass('eh-preview');
  });
});

var inCont = 0;
var inputsArray = {};
var outCont = 0;
var outputsArray = {};
var constCont = 0;
var constArray = {};

function setInId(id){
    if(!(id in inputsArray)){
        inputsArray[id] = inCont++;
    }
    return inputsArray[id];
}
function setOutId(id){
    if(!(id in outputsArray)){
        outputsArray[id] = outCont++;
    }
    return outputsArray[id];
}
function setCId(id){
    if(!(id in constArray)){
        constArray[id] = constCont++;
    }
    return constArray[id];
}

cy.on('remove', 'node', evt => {
    const id = evt.target.id();
    const type = evt.target.data().type;
    if(id in inputsArray){
        var v = inputsArray[id];
        delete inputsArray[id];
        inCont--;
        for(var k in inputsArray){
            if(inputsArray[k] > v){
                inputsArray[k] = inputsArray[k]-1;
            }
         }
    }else if(id in outputsArray){
        var v = outputsArray[id];
        delete outputsArray[id];
        outCont--;
        for(var k in outputsArray){
            if(outputsArray[k] > v){
                outputsArray[k] = outputsArray[k]-1;
            }
        }
    }else if(id in constArray){
        var v = constArray[id];
        delete constArray[id];
        constCont--;
        for(var k in constArray){
            if(constArray[k] > v){
                constArray[k] = constArray[k]-1;
            }
        }
    }
    cy.nodes().forEach(elem => {
       elem.data('label','');
    });
});

cy.nodeHtmlLabel([
  {
    query: '.input',
    cssClass: 'iopbox',
    tpl(data) {
        data.in_id = setInId(data.id);
        return `<a style="font-weight: bold; font-size:12px;">IN`+data.in_id+`</a>`;
    },
  },
  {
    query: '.output',
    cssClass: 'iopbox',
    tpl(data) {
        data.out_id = setOutId(data.id);
        return `<a style="font-weight: bold; font-size:12px;">OUT`+data.out_id+`</a>`;
    },
  },
  {
    query: '.binary',
    cssClass: 'iopbox',
    tpl(data) {
      switch (data.type) {
        case "add":
          return `<a style="font-weight: bold;font-size:12px;">ADD</a>`;
        case "sub":
          return `<a style="font-weight: bold;font-size:12px;">SUB</a>`;
        case "mult":
          return `<a style="font-weight: bold;font-size:12px;">MULT</a>`;
        case "and":
          return `<a style="font-weight: bold;font-size:12px;">AND</a>`;
        case "or":
          return `<a style="font-weight: bold;font-size:12px;">OR</a>`;
        case "xor":
          return `<a style="font-weight: bold;font-size:12px;">XOR</a>`;
        case "min":
          return `<a style="font-weight: bold;font-size:12px;">MIN</a>`;
        case "max":
          return `<a style="font-weight: bold;font-size:12px;">MAX</a>`;
        case "slt":
          return `<a style="font-weight: bold;font-size:12px;">SLT</a>`;
        default:
          return `<a style="font-weight: bold;font-size:12px;"></a>`;
      } 
    },
  },
  {
    query: '.unary',
    cssClass: 'iopbox',
    tpl(data) {
      switch (data.type) {
        case "reg":
          return `<a style="font-weight: bold;font-size:12px;">REG</a>`;
        case "abs":
          return `<a style="font-weight: bold;font-size:12px;">ABS</a>`;
        case "not":
          return `<a style="font-weight: bold;font-size:12px;">NOT</a>`;
        case "const":
            data.cid = setCId(data.id);
            return `<a style="font-weight: bold; font-size:12px;">C`+data.cid+`</a>`;
        default:
          return `<a style="font-weight: bold;font-size:12px;"></a>`;
      } 
    },
  },
]);

export { cy, nodeTypes };

