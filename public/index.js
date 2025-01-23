document.addEventListener('DOMContentLoaded', function() {
    console.log('Nginx test page loaded successfully!');
    
    updateCurrentTime();
    setInterval(updateCurrentTime, 1000);
    
    const testButton = document.getElementById('test-button');
    let clickCount = 0;
    
    testButton.addEventListener('click', function() {
        clickCount++;
        document.getElementById('click-count').textContent = `Button clicks: ${clickCount}`;
        
        const randomColor = getRandomColor();
        testButton.style.backgroundColor = randomColor;
    });
    
    setTimeout(function() {
        document.body.classList.add('js-loaded');
        console.log('JavaScript execution confirmed');
    }, 1000);
});

function updateCurrentTime() {
    const now = new Date();
    const timeString = now.toLocaleTimeString();
    document.getElementById('current-time').textContent = timeString;
}

function getRandomColor() {
    const letters = '0123456789ABCDEF';
    let color = '#';
    for (let i = 0; i < 6; i++) {
        color += letters[Math.floor(Math.random() * 16)];
    }
    return color;
}

console.log('index.js loaded successfully!');
