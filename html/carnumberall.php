<?php
    include('dbcon.php');
?>

    <table class="table table-bordered table-hover table-striped" style="table-layout: fixed">  
        <thead>  
        <tr>  
            <th>차량번호</th>
            <th>전화번호</th>
		<tr>	
		</thead> 
		<?php  
		$stmt = $con->prepare('SELECT carnumber, phonenumber FROM users ORDER BY carnumber DESC');
	    $stmt->execute();
		
            if ($stmt->rowCount() > 0)
            {
                while($row=$stmt->fetch(PDO::FETCH_ASSOC))
	        {
		    extract($row);
        ?>
		<tr>  
			<td><?php echo $carnumber;  ?></td> 
			<td><?php echo $phonenumber; ?></td>
			</tr> 
		<?php
                }
             }
        ?>  

        </table>  
</div>

</body>
</html>