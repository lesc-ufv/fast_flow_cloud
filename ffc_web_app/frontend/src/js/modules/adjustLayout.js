export default function adjustLayout(cy) {
  cy.resize();
  cy.layout({
    name: 'dagre',
    animate: true,
    fit: false,
  }).run();
}
