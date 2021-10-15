export default function (cy) {
  const ur = cy.undoRedo({
    undoableDrag: false,
  });

  document.addEventListener('keydown', e => {
    if (e.ctrlKey && e.target.nodeName === 'BODY') {
      if (e.which === 67) {
        // CTRL + C
        cy.clipboard().copy(cy.$(':selected'));
      } else if (e.which === 86) {
        // CTRL + V
        ur.do('paste');
      } else if (e.which === 65) {
        cy.elements().select();
        e.preventDefault();
      }
    }
  });
}
