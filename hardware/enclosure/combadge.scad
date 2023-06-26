$wallThickness = 1;
$wiggleRoom = 0.5;
$chamberDepth = 10;

$pcbMajorAxis = 45;
$pcbMinorAxis = 31;

$chevronHeight = 13;

$fn = 64;

module chevron() {
    linear_extrude($chevronHeight) import("chevron.svg");
}

module combadge() {
    $cavityLength = $pcbMajorAxis + 2*$wiggleRoom;
    $cavityWidth = $pcbMinorAxis + 2*$wiggleRoom;
    $baseLength = $cavityLength + 2*$wallThickness;
    $baseWidth = $cavityWidth + 2*$wallThickness;
    
    difference() {
        union() {
            translate([6, -2, 0]) scale([0.85, 0.85, 1]) chevron();
            translate([$baseLength/2, $baseWidth/2, 0]) scale([$baseLength/$baseWidth, 1, 1])
                cylinder($chamberDepth + $wallThickness, d = $baseWidth);
        }
        translate([$baseLength/2, $baseWidth/2, 0]) scale([$cavityLength/$cavityWidth, 1, 1])
            cylinder($chamberDepth, d = $cavityWidth);
    }
}

combadge();