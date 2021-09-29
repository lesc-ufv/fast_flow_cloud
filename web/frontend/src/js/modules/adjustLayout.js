export default function adjustLayout(cy) {
  cy.resize();
  cy.layout({
    name: 'cola',
    animate: true,
    fit: false,
  }).run();
}
